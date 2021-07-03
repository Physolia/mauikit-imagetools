//
// Created by gabridc on 5/6/21.
//
#include "cities.h"

#include <QStringList>

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>
#include <QFile>
#include <QCoreApplication>

Cities *Cities::m_instance = nullptr;

static QString resolveDBFile()
{
#if defined(Q_OS_ANDROID)

    QFile file(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "cities.db"));

    if(!file.exists())
    {
        if(QFile::copy(":/android_rcc_bundle/qml/org/mauikit/imagetools/cities.db", QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/org/mauikit/imagetools/cities.db"))
        {
            qDebug() << "Cities DB File was copied to";
        }
    }
    return  QStandardPaths::locate(QStandardPaths::GenericDataLocation, "cities.db");
#else
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, "/org/mauikit/imagetools/cities.db");
#endif
}

const static QString DBFile = resolveDBFile();

Cities::Cities(QObject * parent) : QObject(parent)
{
    qDebug() << "Setting up Cities instance";

    connect(qApp, &QCoreApplication::aboutToQuit, [this]()
    {
        qDebug() << "Lets remove Tagging singleton instance";

        qDeleteAll(m_dbs);
        m_dbs.clear();

        delete m_instance;
        m_instance = nullptr;
    });

    parseCities();
}

Cities::~Cities()
{

}

bool CitiesDB::error() const
{
    return m_error;
}

const City Cities::findCity(double latitude, double longitude)
{
    qDebug() << "Latitude: " << latitude << "Longitud: " << longitude;
    auto pointNear = m_citiesTree.nearest_point({latitude, longitude});
    qDebug()  << pointNear[0] << pointNear[1];

   return db()->findCity(pointNear[0], pointNear[1]);
}

const City Cities::city(const QString &id)
{
    return db()->city(id);
}

void Cities::parseCities()
{    
    if(Cities::m_pointVector.empty())
    {
        qDebug() << "KDE TREE EMPTY FILLING IT";

        Cities::m_pointVector = db()->cities();
        Cities::m_citiesTree = KDTree(Cities::m_pointVector);

        emit citiesReady();
    }
}

CitiesDB *Cities::db()
{
    if(m_dbs.contains(QThread::currentThreadId()))
    {
        qDebug() << "Using existing CITIESDB instance" << QThread::currentThreadId();

        return m_dbs[QThread::currentThreadId()];
    }

    qDebug() << "Creating new CITIESDB instance" << QThread::currentThreadId();

    auto new_db = new CitiesDB;
    m_dbs.insert(QThread::currentThreadId(), new_db);
    return new_db;
}

CitiesDB::CitiesDB(QObject *)
{
    if(QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE")))
    {
        qDebug() << "opening Cities DB";
        m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QUuid::createUuid().toString());

        m_db.setDatabaseName(DBFile);
        qDebug() << "Cities DB NAME" << m_db.connectionName();

        if(!m_db.open())
        {
            qWarning() << "Cities::DatabaseConnect - ERROR: " << m_db.lastError().text();
            m_error = true;
        }else
        {
            m_error = false;
        }
    }
    else
    {
        qWarning() << "Cities::DatabaseConnect - ERROR: no driver " << QStringLiteral("QSQLITE") << " available";
        m_error = true;
    }
}

const City CitiesDB::findCity(double latitude, double longitude)
{
    if(m_error)
    {
        return City();
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM CITIES where lat = ? and lon = ?");
    query.addBindValue(latitude);
    query.addBindValue(longitude);

    if(!query.exec())
    {
        qWarning() << "Cities::FindCity - ERROR: " << query.lastError().text();
    }

    if(query.first())
    {
        return City(query.value("id").toString(), query.value("name").toString(), query.value("tz").toString(), query.value("country").toString(),query.value("lat").toDouble(),query.value("lon").toDouble());
    }

    qWarning() << "City not found";

    return City();
}

const City CitiesDB::city(const QString &cityId)
{
    if(m_error)
    {
        return City();
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT c.id, c.name, co.name as country, c.lat, c.lon FROM CITIES c inner join COUNTRIES co on c.country = co.id where c.id = ?");
    query.addBindValue(cityId);

    if(!query.exec())
    {
        qWarning() << "Cities::city - ERROR: " << query.lastError().text();
    }

    if(query.first())
    {
        return City(query.value("id").toString(), query.value("name").toString(), query.value("tz").toString(), query.value("country").toString(),query.value("lat").toDouble(),query.value("lon").toDouble(), this);
    }

    return City();
}

std::vector< point_t > CitiesDB::cities()
{
    std::vector< point_t >  res;
    QSqlQuery query(m_db);
    query.prepare("SELECT lat, lon FROM CITIES");

    if(!query.exec())
    {
        qWarning() << "Cities::ParsingCities - ERROR: " << query.lastError().text();
        m_error = true;
    }

    while(query.next())
    {
        double lat = query.value("lat").toDouble();
        double lon = query.value("lon").toDouble();
        res.push_back({lat, lon});
    }

     m_error = false;
    return res;
}
