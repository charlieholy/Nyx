{
    "rabbitmqclient":{
        "url":"amqp://username:password@IP:PORT",
        "product":{
            "exchange":"exchange.trade.depth",
            "routing":"queue.nyx.combo",
            "queue":"queue.nyx.combo"
        },
        "custom":{
            "exchange":"exchange.trade.depth",
            "routing":"queue.combo.nyx",
	    "queue":"queue.combo.nyx"
        }

    },
    "wsserverRpc": {
        "bind": "tcp@0.0.0.0:12389",
        "max_pkg_size": 102400,
        "protocol": "chat"
    },
    "tcpserverRpc": {
        "bind": "tcp@0.0.0.0:12388",
        "max_pkg_size": 102400
    }
}

