# Simple_NAT_Punching
Данный код реализует защищенное NAT Punching. Этот алгоритм позваляет общаться клиенту и серверу передавая сообщения напрямую друг другу. Данный пример учебный, и испльзует некоторые упращения. Мы будем рассматривать систему, в котрой клиент и сервер находятся в разных сетях, отправлять будем единственное сообщение - число от клиента и ответ от сервера.
# Конфигурация Системы и NatPunching.gns3project
Конфигурация и тесты проводились в GNS3. Наша система выглядит так :
![image](https://github.com/denizzzzka/Simple_NAT_Punching/assets/91347518/41fa067b-87c0-4680-a929-fc509444b73f)
Здесь client и server это два взаимодействующих обьекта. В даннам стенде route-1 и route-2 эмулируют работу роутеров, для этого необходимо прописать для корректной эмуляции :  
`iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE \n 
iptables -A INPUT -i eth0 -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A INPUT -i eth0 -j DROP`  
helpserver - это вспомогательный сервер, с помощью которого происходит обмен глобальными адрессами. NAT в этой системе нужен для подгрузки gcc, iptables и т.д.
Для корректной работы стенда, после подгрузки всего необходимого, следует установить маршруты по умолчанию для server и client. Данный проэкт находится в файле NatPunching.gns3project 

# Компиляция  
` gcc -o main main.c -L/usr/lib -lssl -lcrypto`  
# Пример файла Keys.txt  
![image](https://github.com/denizzzzka/SSL-client-server/assets/91347518/9f3a2bec-5c2c-4753-852d-74aab5e19ab5)  
# Пример зашифрованных сообжений и дешифровки  
Дешифровка выполняется с помощью сертификата и файла Keys.txt  
Зашифрованные данные :  
![image](https://github.com/denizzzzka/SSL-client-server/assets/91347518/cf5cb6e7-bd1d-408e-bd12-12868e1d7b31)  
Дешифрованные : 
![image](https://github.com/denizzzzka/SSL-client-server/assets/91347518/a732ef9c-1b57-4896-9a62-e6a437680776)
