#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>

#include "mqtt_adapter/mqttadapter.h"

const double HYSTERISIS = 0.3;

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    void callForHeat(bool value = true);

private:

    void decideHeat();
    bool heating;
    bool heatingKnown;

    MqttAdapter *adapter;

    double currentTemperature;
    QDateTime currentTemperatureAge;

    double temperatureSetpoint;

    QDateTime callForHeatTime;

    QTimer *currentTemperatureReceiveTimer;

signals:

public slots:

    void on_mqttConnected();
    void on_mqttMessageReceived(QMQTT::Message message);
    void on_currentTemperatureReceiveTimerTimeout();
};

#endif // CONTROLLER_H
