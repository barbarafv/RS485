# RS485

O projeto consiste na  conexão entre o elemento mestre e o PC por uma rede Wifi, promovendo o controle e as leituras de sensores de 2 escravos através de uma interface web HTTP básica. 
Por exemplo, ao clicar sobre o botão “Desligar LED” do slave 2 a
página irá abrir uma nova aba com o endereço “192.168.137.113/desligaled2”. A
extensão após o endereço IP mudará para cada tipo de serviço.
Assim será feito uma solicitação ao mestre, que identificará o processo através
da extensão do IP, fará a busca dos dados e então retornará para a página aberta a resultado da solicitação. 
