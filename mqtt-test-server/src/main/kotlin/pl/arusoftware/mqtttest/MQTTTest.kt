package pl.arusoftware.mqtttest

import pl.arusoftware.mqtttest.mqtt.Connection

fun main() {
    val mqttConnection = Connection.create("localhost")
    mqttConnection.subscribeTopic("adevice/+/data") { topic, message ->
        println("$topic = ${String(message.payload)}")
    }
    while (true) {
        mqttConnection.publishMessage("adevice/NMCU_1589714856299/settings", "test message")
        Thread.sleep(1000)
    }
}