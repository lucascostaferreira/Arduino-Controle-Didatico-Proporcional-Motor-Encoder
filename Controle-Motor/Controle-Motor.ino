//definindo pino digital para a saída do encoder
#define ENC 8
//definindo pino PWM para o gate do MOSFET
#define GATE 10
//definindo quantidade de "dentes" do encoder
#define DENTES 32

//declarando variáveis globais
int resposta, referencia, atuacao;
float ganho, freq;

//declarando função reset
void(* reset) (void) = 0;
//declarando função de cálculo do periodo
unsigned long periodo(int pin);

void setup(){
  //configurando o pino digital para a saída do encoder como pino de entrada
  pinMode(ENC, INPUT);
  //configurando o pino PWM para o gate do MOSFET como pino de saída
  pinMode(GATE, OUTPUT);
  
  //abrindo o MOSFET para garantir que o motor permaneça desligado
  digitalWrite(GATE, LOW);
  
  //iniciando comunicação serial entre o Arduino e o computador
  Serial.begin(2000000);
  //aguardando sinal para iniciar as leituras
  while(!(Serial.available() > 0)){
    delay(100);
  }

  //declarando variáveis auxiliares
  unsigned short aux1, aux2;
  //recebendo os valores para a referencia e para o ganho
  sscanf(Serial.readStringUntil(' ').c_str(), "%i", &referencia);
  sscanf(Serial.readStringUntil(' ').c_str(), "%hu", &aux1);
  sscanf(Serial.readStringUntil('\0').c_str(), "%hu", &aux2);
  //limpando sinal serial recebido
  while(Serial.available() > 0){
    Serial.read();
  }
  
  //calculando ganho
  ganho = aux1+aux2/100;
}

void loop(){
  //estimando a frequência do encoder em Hz
  freq = 1000000.0/periodo(ENC);
  //realizando cálculos para converter a frequência do encoder em RPM
  resposta = freq*(60.0/DENTES);

  //calculando sinal de atuação
  atuacao = ganho*(referencia-resposta);
  //atuando sobre o motor
  if(atuacao < 0)
    analogWrite(GATE, 0);
  else{
    if(atuacao > 255)
      analogWrite(GATE, 255);
    else
      analogWrite(GATE, atuacao);
  }
  
  //enviando os dados ao computador
  Serial.println(resposta);
  
  //verificando mudança de referência
  if(Serial.available() > 0){
    //muda o valor de referência
    sscanf(Serial.readStringUntil('\0').c_str(), "%i", &referencia);
    //verificando ordem de reset
    if(referencia == 0){
      //para a comunicação serial entre o Arduino e o computador
      Serial.end();
      //reseta o Arduino
      reset();
    }
    //limpando sinal serial recebido
    while(Serial.available() > 0){
      Serial.read();
    }
  }
}

unsigned long periodo(int pin){
  unsigned long t0, ts = micros();
  while(digitalRead(pin) == 1){
    if(micros()-ts > 1000000)
      return 10000000;
  }
  while(digitalRead(pin) == 0){
    if(micros()-ts > 1000000)
      return 10000000;
  }
  //inicia a contagem de tempo
  t0 = micros();
  while(digitalRead(pin) == 1){
    if(micros()-ts > 1000000)
      return 10000000;
  }
  while(digitalRead(pin) == 0){
    if(micros()-ts > 1000000)
      return 10000000;
  }
  //termina a contagem de tempo e retorna o tempo de um período em microsegundos
  return micros()-t0;
}
