package pl.arusoftware.mqtttest

import pl.arusoftware.mqtttest.mqtt.Connection

fun main() {
    val mqttConnection = Connection.create("localhost")
    mqttConnection.subscribeTopic("adevice/+/data") { topic, message ->
        println("$topic = ${String(message.payload)}")
    }
    println("In main function")
}