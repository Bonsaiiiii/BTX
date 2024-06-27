A SSID e senha são enviadas ao servidor, lá é validado as variáveis são analisadas e se estiverem corretas são armazenadas, caso contrário elas são descartadas, caso corretas é alterado o JSON
que tem o valor bool armazenando o status da rede (ex: caso conectado: 1) esse JSON é utilizado pelo JS onde ele é análisado e ao apertar no botão de input é análisado os bools de conectado e erro
caso o bool conectado for true, uma mensagem de bem-sucedido é enviada, caso a bool erro for true ele mostra uma mensagem de erro. 
