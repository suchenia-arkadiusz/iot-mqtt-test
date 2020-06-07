package pl.arusoftware.mqtttest.mqtt

import org.eclipse.paho.client.mqttv3.*
import java.util.*

class Connection private constructor(val publisherId: String, val uri: String, private val mqttClient: IMqttClient){
    companion object {
        fun create(uri: String, port: Int = 1883, publisherId: String = UUID.randomUUID().toString()): Connection {

            val mqttClient = MqttClient("tcp://$uri:$port", publisherId)

            val connectOptions = MqttConnectOptions()
            connectOptions.isAutomaticReconnect = true
            connectOptions.isCleanSession = true
            connectOptions.connectionTimeout = 10
            mqttClient.connect(connectOptions)

            return Connection(publisherId, uri, mqttClient)
        }
    }

    private val usedTopics = mutableListOf<String>()

    fun subscribeTopic(topic: String, listener: (topic: String, message: MqttMessage) -> Unit) {
        usedTopics.add(topic)
        mqttClient.subscribe(topic, listener)
    }

    fun publishMessage(topic: String, message: String) {
        val mqttMessage = MqttMessage(message.toByteArray())
        mqttClient.publish(topic, mqttMessage)
    }

    fun unsubscribe(topic: String) {
        mqttClient.unsubscribe(topic)
    }

    fun unsubscribeAll() {
        mqttClient.unsubscribe(usedTopics.toTypedArray())
    }
}