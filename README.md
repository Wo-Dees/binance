# Binana = Binance Connector

Мой прототип коннектора в Binance для спотовой торговли.

Реализован функционал для установки лимитного/рыночного ордера, их отмены и наблюдения за рынком.

Как запустить? 

1. Сгенерировать ключ и токен на официальном сайте (можно для testnet).
2. Установить переменные окружения 
'''
export base_url='testnet.binance.vision'
export binance_testnet_api_key='{your_api_key}'
export binance_testnet_secret_key='{your_secret_key}'
'''