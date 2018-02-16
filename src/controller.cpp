#include "controller.h"

#include <QDebug>

Controller::Controller(QObject *parent) : QObject(parent)
{

    adapter = new MqttAdapter(this);

    currentTemperatureReceiveTimer = new QTimer();
    currentTemperatureReceiveTimer->start(60 * 1000);

    connect(currentTemperatureReceiveTimer, SIGNAL(timeout()),
            this, SLOT(on_currentTemperatureReceiveTimerTimeout()));
    connect(adapter, SIGNAL(mqttMessageReceived(QMQTT::Message)),
            this, SLOT(on_mqttMessageReceived(QMQTT::Message)));
    connect(adapter, SIGNAL(mqttConnected()),
            this, SLOT(on_mqttConnected()));

    heating = false;
    heatingKnown = false;

    temperatureSetpoint = -100;

    adapter->start();

}

void Controller::callForHeat(bool value)
{
    if (value) {
        qDebug() << qPrintable(QString("%1 START heat; temp = %2")
                               .arg(QDateTime::currentDateTime().toString())
                               .arg(currentTemperature));
        adapter->mqttPublish("gpio/in/0",QString("1"));
        callForHeatTime = QDateTime::currentDateTime();
        heating = true;
    } else {
        qDebug() << qPrintable(QString("%1 END heat; temp = %2 (%3)")
                               .arg(QDateTime::currentDateTime().toString())
                               .arg(currentTemperature)
                               .arg((QDateTime::currentDateTime().toMSecsSinceEpoch() - callForHeatTime.toMSecsSinceEpoch()) / 1000));
        adapter->mqttPublish("gpio/in/0",QString("0"));
        heating = false;
    }

}

void Controller::decideHeat()
{
    // failsafe
    if (((QDateTime::currentDateTime().toMSecsSinceEpoch() - currentTemperatureAge.toMSecsSinceEpoch()) / 1000) > (60 * 1000)) {
        qDebug() << qPrintable("FAULT! Temperature too OLD to act!");
        callForHeat(false);
        return;
    }

    // deadband calculations

    if (!heating && (currentTemperature < (temperatureSetpoint - HYSTERISIS))) {
        callForHeat();
    }

    if (heating && (currentTemperature >= (temperatureSetpoint))) {
        callForHeat(false);
    }

}

void Controller::on_mqttConnected()
{
    qDebug() << "controller connected to mqtt broker";
    adapter->mqttSubscribe("rht03/0/temp");
    adapter->mqttSubscribe("hvac/temp_setpoint");
}

void Controller::on_mqttMessageReceived(QMQTT::Message message)
{
    if (message.topic() == "hvac/temp_setpoint") {
        temperatureSetpoint = message.payload().toDouble();
        qDebug() << qPrintable(QString("Received Temp Setpoint of %1").arg(temperatureSetpoint));
    }

    if (message.topic() == "rht03/0/temp") {
        if (message.payload().length() > 0) {
            currentTemperature = message.payload().toDouble();
            currentTemperatureAge = QDateTime::currentDateTime();
            currentTemperatureReceiveTimer->start(60 * 1000);
            decideHeat();
        }

    }
}

void Controller::on_currentTemperatureReceiveTimerTimeout()
{
    callForHeat(false);
    qDebug() << qPrintable(QString("%1 FAULT - temp receive timeout").arg((QDateTime::currentDateTime().toMSecsSinceEpoch()/1000)));
}
