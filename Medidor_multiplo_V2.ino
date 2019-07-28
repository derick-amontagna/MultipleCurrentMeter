// --- Bibliotecas auxiliares ---
#include <LiquidCrystal_SoftI2C.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ACS712.h>

// --- Definindo parâmetros ---
//Display 16x2
// Set SDA to pin 3 and SCL to pin 2
SoftwareWire *wire = new SoftwareWire(3, 2);
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2, wire);

//Sensores de corrente
#define ACS712_PIN_A0 A0
#define ACS712_PIN_A1 A1
#define ACS712_PIN_A2 A2
#define ACS712_PIN_A3 A3
#define ACS712_PIN_A4 A4
#define ACS712_PIN_A5 A5

ACS712 sensor0(ACS712_20A, A0);
ACS712 sensor1(ACS712_20A, A1);
ACS712 sensor2(ACS712_20A, A2);
ACS712 sensor3(ACS712_20A, A3);
ACS712 sensor4(ACS712_20A, A4);
ACS712 sensor5(ACS712_20A, A5);

//Modulo de cartão SD
#define chipSelect     10               //Comunicação SPI, chipSelect  no digital 10

/*
  VCC do módulo para 5V do Arduino.
  GND do módulo para GND do Arduino.
  CS para pino digital 10 da Uno.
  MOSI para pino digital 11 da Uno.
  MISO para o pino digital 12.
  SCK para o pino digital 13.
*/

//Botões
#define butOk 4
#define butUp 5
#define butDown 6
#define butStop 7

//Sensor de temperatura
#define ONE_WIRE_BUS 8          //sinal do sensor DS18B20

// --- Declaração de Objetos ---
OneWire oneWire(ONE_WIRE_BUS);        //objeto one_wire
DallasTemperature sensors(&oneWire);
DeviceAddress sensor12;


// --- Variaveis Globais ---
boolean t_reset = 0x00, t_Stop = 0x00;
char sensores;           //Variavel contabilizadora do numero de sensores no menu
char mod;               //Variavel determinadora do modo de execução
boolean t_butUp, t_butDown, t_butOk;            //Flags para armazenar o estado dos botões
long tempo1, tempo2;
int  counter;                                                // Variável auxiliar do temporizador para realizar as medições no tempo determinado
int  tempo_total;                                           // Variável do temporizador, contabilizando o tempo total de executação das medições

// --- Funções Auxiliares ---

//Menu
char Muda_valor_ok (char FOk)                                       //Modifica o valor atual do botão ok
{

  if (!digitalRead(butOk)) t_butOk = 0x01;         //Botão Ok pressionado? Seta flag

  if (digitalRead(butOk) && t_butOk)                   //Botão Ok solto e flag setada?
  { //Sim...
    t_butOk = 0x00;                                   //Limpa flag
    lcd.clear();
    FOk++; //Incrementa Ok

  } //end butOk

  return FOk;
}//end Muda_valor_ok

char modo(char Fmod)                           //Função que seleciona o modo de execução do medidor
{
  lcd.setCursor(0, 0);
  lcd.print("Modo: 0 - Teste");
  lcd.setCursor(6, 1);
  lcd.print("1 - Logger");
  lcd.setCursor(0, 1);
  lcd.print("Op:");
  lcd.setCursor(4, 1);
  lcd.print(Fmod, 10);
}

char Muda_valor_mod(char Fmod)                                      //Modifica o valor atual dos botões up ou down
{
  if (!digitalRead(butUp))   t_butUp   = 0x01;         //Botão Up pressionado? Seta flag
  if (!digitalRead(butDown)) t_butDown = 0x01;         //Botão Down pressionado? Seta flag

  if (digitalRead(butUp) && t_butUp)                   //Botão Up solto e flag setada?
  { //Sim...
    t_butUp = 0x00;                                   //Limpa flag
    Fmod++;    //Incrementa segundo
    if (Fmod > 1)
    {
      Fmod = 0;                     //Se segundo for maior que 59, volta a ser 0
    }
  }
  else if (digitalRead(butDown) && t_butDown)               //Botão Down solto e flag setada?
  { //Sim...
    t_butDown = 0x00;                                 //Limpa flag
    Fmod--;    //Incrementa segundo
    if (Fmod < 0)
    {
      Fmod = 1;                     //Se segundo for maior que 59, volta a ser 0
    }

  }
  return Fmod;
}//end Muda_valor_mod

char Muda_valor_Fn (char Fn)                                      //Modifica o valor atual dos botões up ou down
{
  if (!digitalRead(butUp))   t_butUp   = 0x01;         //Botão Up pressionado? Seta flag
  if (!digitalRead(butDown)) t_butDown = 0x01;         //Botão Down pressionado? Seta flag

  if (digitalRead(butUp) && t_butUp)                   //Botão Up solto e flag setada?
  { //Sim...
    t_butUp = 0x00;                                   //Limpa flag
    Fn++;    //Incrementa segundo
    if (Fn > 59)
    {
      lcd.clear();
      Fn = 0;                     //Se segundo for maior que 59, volta a ser 0
    }
  }
  else if (digitalRead(butDown) && t_butDown)               //Botão Down solto e flag setada?
  { //Sim...
    t_butDown = 0x00;                                 //Limpa flag
    lcd.clear();
    Fn--;    //Decrementa segundo
    if (Fn < 0) Fn = 59;                     //Se segundo for menor que 0, volta a ser 59
  }
  return Fn;
}//end Muda_valor_Fn

char Muda_valor_Sensores (char sen)                                      //Modifica o valor atual dos botões up ou down
{
  if (!digitalRead(butUp))   t_butUp   = 0x01;         //Botão Up pressionado? Seta flag
  if (!digitalRead(butDown)) t_butDown = 0x01;         //Botão Down pressionado? Seta flag

  if (digitalRead(butUp) && t_butUp)                   //Botão Up solto e flag setada?
  { //Sim...
    t_butUp = 0x00;                                   //Limpa flag
    sen++;    //Incrementa sensores
    if (sen > 6) sen = 1;                     //Se sensores for maior que 6, volta a ser 1
  }
  else if (digitalRead(butDown) && t_butDown)               //Botão Down solto e flag setada?
  { //Sim...
    t_butDown = 0x00;                                 //Limpa flag
    lcd.clear();
    sen--;    //Decrementa sensores
    if (sen < 1) sen = 6;                     //Se sensores for menor que 1, volta a ser 6
  }
  return sen;
}//end Muda_valor_sensores

void dispMenu(char FOk, char Fsegundo, char Fminuto, char Fnum_sen)                                         //Mostra o menu atual
{
  switch (FOk)                                       //Controle da variável menu
  {
    case 0x01:                                       //Caso 1
      tempo(FOk, Fsegundo, Fminuto);                               //Chama a função de tempo

      break;                                     //break

    case 0x02:                                       //Caso 1
      tempo(FOk, Fsegundo, Fminuto);                               //Chama a função de tempo

      break;                                     //break
    case 0x03:                                       //Caso 2
      num_sensores(Fnum_sen);                             //Chama a função do num_sensores

      break;                                     //break

  } //end switch menu
}//end dispMenu

void tempo(char FFOk, char FFsegundo, char FFminuto)                                        //Função que mostra a seleção do minuto e segundo entre as medições
{
  lcd.setCursor(0, 0);
  lcd.print("Tempo entre");
  lcd.setCursor(0, 1);
  lcd.print("medicoes:");
  if (FFOk == 1)
  {
    lcd.setCursor(13, 0);
    lcd.print("seg");
  }
  else if (FFOk == 2)
  {
    lcd.setCursor(13, 0);
    lcd.print("min");
  }
  lcd.setCursor(13, 1);
  lcd.print(FFsegundo, 10);
  lcd.setCursor(12, 1);
  lcd.print(":");
  lcd.setCursor(10, 1);
  lcd.print(FFminuto, 10);

} //end tempo

void num_sensores(char Fsensores)                           //Função que seleciona o numero de sensores a realizarem as medida
{
  lcd.setCursor(0, 0);
  lcd.print("Numero de sensor:");
  lcd.setCursor(6, 1);
  lcd.print(Fsensores, 10);

} //end num_sensores()

boolean Muda_valor_Stop (boolean FStop)                                      //Modifica o valor atual do botão Stop
{

  if (!digitalRead(butStop)) t_Stop = 0x01;         //Botão Stop pressionado? Seta flag

  if (digitalRead(butStop) && t_Stop)                   //Botão Stop solto e flag setada?
  { //Sim...
    t_Stop = 0x00;                                   //Limpa flag
    lcd.clear();
    if (!FStop)
    {
      FStop = 1;  //Ativa
    }
    else
    {
      FStop = 0;  //Desativa
    }

  } //end butStop

  return FStop;
}//end Muda_valor_Stop

// --- Inicializando ---
int IniciandoMedidor(void)
{
  //Criando variaveis
  int Fespelho = 0;
  mod = 0;
  boolean passa = 0x00;
  t_butUp    = 0x00;                                      //limpa flag do botão Up
  t_butDown  = 0x00;                                      //limpa flag do botão Down
  t_butOk    = 0x00;                                     //limpa flag do botão OK
  char Ok = 0;                //Variavel contabilizadora dos Ok's do menu
  char minuto = 0;              //Variavel contabilizadora do minuto no menu
  char segundo = 0;            //Variavel contabilizadora do segundo no menu
  sensores = 1;           //Variavel contabilizadora do numero de sensores no menu
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0, 0);
  lcd.print("Inicializando o");
  lcd.setCursor(5, 1);
  lcd.print("Medidor");
  delay(2000);
  lcd.clear();

  //Menus
  while (Ok == 0) //loop do modo de execução
  {
    modo(mod);
    mod = Muda_valor_mod(mod);
    Ok = Muda_valor_ok (Ok);
  }
  while (Ok < 4) //loop do menu
  {
    if (Ok == 1)
    {
      segundo = Muda_valor_Fn(segundo);
    }
    else if (Ok == 2)
    {
      minuto = Muda_valor_Fn(minuto);

    }
    else if (Ok == 3)
    {
      sensores = Muda_valor_Sensores(sensores);

    }
    Ok = Muda_valor_ok (Ok);
    dispMenu(Ok, segundo, minuto, sensores);
  }


  //Operações
  if (mod == 1) //Se o modo selecionado for o datalogger, ele inicia e cria o arquivo
  {
    //Cartão SD
    while (!Serial)
    {
      // Aguardando a porta serial se conectar. Necessário apenas para porta USB nativa.
    }

    // Inicializa o modulo SD
    if (!SD.begin(chipSelect))
      //if (false)
    {
      //Serial.print("Falha ao acessar o cartao!");
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Falha ao acessar");
      lcd.setCursor(0, 1);
      lcd.print("o cartao!");
      delay(10000);
      //tela.print("Verifique o cartao/conexoes e reinicie o Arduino...",LEFT,24);
      // O programa para por aqui.
      t_reset   = 0x01;
      passa = 0x01;
      lcd.clear();

    }
    else
    {
      //Serial.println("Cartao inicializado!");
      //Serial.println();
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(5, 0);
      lcd.print("Cartao");
      lcd.setCursor(2, 1);
      lcd.print("inicializado!");
      delay(1000);
      lcd.clear();
    }
    //Cria e escreve arquivo CSV
    File logFile = SD.open("LOG5.csv", FILE_WRITE);
    if (logFile)
    {
      logFile.println(", , ,"); //
      String header = "tt,S0,S1,S2,S3,S4,S5,temp";
      logFile.println(header); //Adicionando o cabeçalho do datalogger
      logFile.println("0, 0, 0, 0, 0, 0, 0, 0"); //Iniciando os dados
      logFile.close();
      //Serial.println(header);
      //Serial.println("0, 0, 0, 0, 0, 0, 0, 0");
      lcd.setCursor(3, 0);
      lcd.print("DataLogger");
      lcd.setCursor(4, 1);
      lcd.print("Iniciado");
      delay(1000);
      lcd.clear();

    } //end if logFile
    else if (!passa)
    {
      lcd.clear();
      //Serial.println("Erro ao abrir arquivo");
      lcd.setCursor(0, 0);
      lcd.print("Erro ao abrir");
      lcd.setCursor(0, 1);
      lcd.print("arquivo!");
      while (1);
    }
  }//end if mod para ativar o datalogger
  Fespelho = segundo + 60 * minuto; //valor a ser contado pelo temporizador
  return Fespelho;
}//end IniciandoMedidor

// --- Tempo ---

void Temporizador_1s()
{
  tempo2 = millis();              //Salva o valor atual do contador de programa em mili segundos
  if (tempo2 - tempo1 > 1000)      //Verificação de tempo
  {
    tempo1 = tempo2;             //Salva o tempo atual
    counter++;                   //Incrementando o contador
    tempo_total++;
  }
}//end Temporizador_1s

// --- Operações de medição

void calibrando(void)
{
  sensor0.setZeroPoint(sensor0.calibrate());
  sensor1.setZeroPoint(sensor1.calibrate());
  sensor2.setZeroPoint(sensor2.calibrate());
  sensor3.setZeroPoint(sensor3.calibrate());
  sensor4.setZeroPoint(sensor4.calibrate());
  sensor5.setZeroPoint(sensor5.calibrate());
}

float MedidorSensores (int i) //Seleciona qual sensor ira realizar a medição e a transforma em corrente (a formula foi deduzida atraves de uma reta)
{
  float somador = 0, medido = 0;
  switch (i)
  {
    case 0:
      medido = sensor0.getCurrentDC();

      return medido;
    case 1:
      medido = sensor1.getCurrentDC();

      return medido;

    case 2:
      medido = sensor2.getCurrentDC();

      return medido;

    case 3:
      medido = sensor3.getCurrentDC();

      return medido;

    case 4:
      medido = sensor4.getCurrentDC();
      return medido;

    case 5:
      medido = sensor5.getCurrentDC();

      return medido;

  } //end switch medição

} //end MedidorSensores

float temp(void)
{
  // Le a informacao do sensor
  sensors.requestTemperatures();
  float TempC = sensors.getTempC(sensor12);
  return TempC;
}

// --- Exibição de valores ---

void MostraNoDisplay (float dado, int localizacao) //Seleciona o que vai ser mostrado no display
{
  switch (localizacao)
  {
    case 0:
      if (dado < 0)
      {
        dado = dado * -1;
      }
      lcd.setCursor(0, 0);
      lcd.print(String(dado, 2));
      lcd.setCursor(4, 0);
      lcd.print("/");

      break;

    case 1:
      if (dado < 0)
      {
        dado = dado * -1;
      }
      lcd.setCursor(5, 0);
      lcd.print(String(dado, 2));
      lcd.setCursor(9, 0);
      lcd.print("/");

      break;

    case 2:
      if (dado < 0)
      {
        dado = dado * -1;
      }
      lcd.setCursor(10, 0);
      lcd.print(String(dado, 2));

      break;

    case 3:
      if (dado < 0)
      {
        dado = dado * -1;
      }
      lcd.setCursor(0, 1);
      lcd.print(String(dado, 2));
      lcd.setCursor(4, 1);
      lcd.print("/");;

      break;

    case 4:
      if (dado < 0)
      {
        dado = dado * -1;
      }
      lcd.setCursor(5, 1);
      lcd.print(String(dado, 2));
      lcd.setCursor(9, 1);
      lcd.print("/");

      break;

    case 5:
      if (dado < 0)
      {
        dado = dado * -1;
      }
      lcd.setCursor(10, 1);
      lcd.print(String(dado, 2));

      break;
  } //end switch display
}//end MostraNoDisplay

// --- Programa principal ---
void setup()
{
  // Inicializações
  Serial.begin(9600);
  lcd.begin();
  sensors.begin();
  for (char i = 4; i < 8; i++) pinMode(i, INPUT_PULLUP); //Entrada para os botões (digitais 4 a 7) com pull-ups internos (Envia zero quando é pressionado)
  pinMode(chipSelect, OUTPUT);                            //Configura o pino CS como saída



  // Localiza e mostra enderecos dos sensores
  Serial.print(sensors.getDeviceCount(), DEC);
  if (!sensors.getAddress(sensor12, 0))
    Serial.println("Sensores nao encontrados !");


}//end setup

void loop()
{
  t_reset = 0x00;
  t_Stop = 0x00;
  int espelho = 0;                             // Variável auxiliar do menu para realizar as medições no tempo determinado
  //Iniciando o medidor
  espelho = IniciandoMedidor();
  //Iniciando Variaveis
  float correntes[6] = { 0 };                                           //Array para armazenar as medições
  tempo1 = 0x00, tempo2 = 0x00;                   // Variável para temporização
  counter = 0;                                                // Variável auxiliar do temporizador para realizar as medições no tempo determinado
  tempo_total = 0;                                           // Variável do temporizador, contabilizando o tempo total de executação das medições
  boolean Stop = 0x00;                                            //Variavel de segurança para a retirada do cartão de memoria
  float tempC = 0;                                                //Variavel para o sensor de temperatura
  while (!t_reset)
  {
    Stop = Muda_valor_Stop(Stop);
    if (Stop)
    {
      lcd.setCursor(2, 0);
      lcd.print("Botao Stop");
      lcd.setCursor(2, 1);
      lcd.print("Pressionado");
      if (!digitalRead(butOk))   t_reset   = 0x01;         //Botão Reset pressionado? Seta flag
    } //end if stop
    else if (!t_reset)
    {
      Temporizador_1s();
      if (espelho == counter) //Quando o temporizador chegar ao tempo de estouro, a medição será realizada
      {
        counter = 0;
        calibrando();
        for (int i = 0; i < sensores; i++) // for para realizar as medições
        {
          correntes[i] = MedidorSensores(i);
        }
        if (mod == 1)
        {
          tempC = temp();
          //Cria string de dados para armazenar no cartão SD
          //Utilizando arquivo do tipo Comma Separete Value
          String dataString = String(tempo_total) + ", " + String(correntes[0]) + ", " + String(correntes[1]) + ", " + String(correntes[2]) + ", " + String(correntes[3]) + ", " + String(correntes[4]) + ", " + String(correntes[5]) + ", " + String(tempC);
          //Abre o arquivo para escrita
          //Apenas um arquivo pode ser aberto de cada vez
          File logFile = SD.open("LOG5.csv", FILE_WRITE);
          if (logFile)
          {
            logFile.println(dataString);
            logFile.close();
            //Serial.println(dataString);
          } //end if logFile
          else
          {
            //Serial.println("Erro ao abrir arquivo para escrita final");
            lcd.setCursor(0, 0);
            lcd.print("Erro ao abrir");
            lcd.setCursor(0, 1);
            lcd.print("Arquivo Final");
          }
        }
        else
        {
          lcd.clear();
        }
      }//end if espelho

      if (mod == 0) //No modo teste
      {
        for (int ii = 0; ii < sensores; ii++) //loop para mostrar no display os valores no array medidor
        {
          MostraNoDisplay(correntes[ii], ii);
        }// end for
        lcd.setCursor(15, 0);
        lcd.print("T");
        lcd.setCursor(15, 1);
        lcd.print(counter, 10);
      }
      else // No modo datalogger
      {
        lcd.setCursor(5, 0);
        lcd.print("Gravando");
        lcd.setCursor(0, 1);
        lcd.print("Tempo Total:");
        lcd.setCursor(12, 1);
        lcd.print(String(tempo_total));
      }
      if (!digitalRead(butOk) && !digitalRead(butStop))   t_reset   = 0x01;         //Botão Reset pressionado? Seta flag
    }//end else stop
  }// end While

  //Zerando valores
  t_reset  = 0x00;

}//end loop
