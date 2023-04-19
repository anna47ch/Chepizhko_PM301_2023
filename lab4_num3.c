// Подключение библиотек
#include "thread.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "msg.h"

uint32_t time = 0;

bool pass = false;



unsigned int count = 1;

unsigned int count2 = 0;

static msg_t rcv_queue[10];

// Выделение памяти под стеки двух тредов
char thread_one_stack[THREAD_STACKSIZE_DEFAULT];
char thread_two_stack[THREAD_STACKSIZE_DEFAULT];

// Глобально объявляется структура, которая будет хранить PID (идентификатор треда)
// Нам нужно знать этот идентификатор, чтобы посылать сообщения в тред
static kernel_pid_t thread_one_pid;

// Обработчик прерывания с кнопки
void btn_handler(void *arg)
{
  // Прием аргументов из главного потока
  (void)arg;

  gpio_read(GPIO_PIN(PORT_C,8));
  msg_t msg;

  if(xtimer_now_usec() >= time + 200000)
  {
  // Создание объекта для хранения сообщения
 
  // Поскольку прерывание вызывается и на фронте, и на спаде, нам нужно выяснить, что именно вызвало обработчик
  // для этого читаем значение пина с кнопкой. Если прочитали высокое состояние - то это был фронт.  
  if (gpio_read(GPIO_PIN(PORT_A,0)) > 0){
    // Отправка сообщения в тред по его PID
    // Сообщение остается пустым, поскольку нам сейчас важен только сам факт его наличия, и данные мы не передаем
     if(count2 < 10){
      count2+=1;
     }
     else{
      count2 = 0;
      count2+=1;
     }
      msg_send(&msg, thread_one_pid);   

    }
    else{
      if(count2 < 10){
        rcv_queue[count2].content.value = (uint32_t)(1);
      }
      else{
          for(unsigned int j = 0; j < 10; j++){rcv_queue[j].content.value = (uint32_t)(0);}
      }
    }

      time = xtimer_now_usec();
  }

}

// Первый поток
void *thread_one(void *arg)
{
  // Прием аргументов из главного потока
  (void) arg;
  // Создание объекта для хранения сообщения 
  msg_t msg;
  // Инициализация пина PC8 на выход
  gpio_init(GPIO_PIN(PORT_C,8),GPIO_OUT);

  while(1){

    // Ждем, пока придет сообщение
    // msg_receive - это блокирующая операция, поэтому задача зависнет в этом месте, пока не придет сообщение
  	msg_receive(&msg);
    // Переключаем светодиод
    for(unsigned int i = 0; i < msg.content.value; i++){

    gpio_set(GPIO_PIN(PORT_C,8));
    xtimer_usleep(333333);
    gpio_clear(GPIO_PIN(PORT_C,8));
    xtimer_usleep(333333);
    }
  }
  // Хотя код до сюда не должен дойти, написать возврат NULL все же обязательно надо    
  return NULL;
}

// Второй поток
void *thread_two(void *arg)
{

  msg_init_queue(rcv_queue, 10);

  

  


  // Прием аргументов из главного потока
  (void) arg;
  // Инициализация пина PC9 на выход
  gpio_init(GPIO_PIN(PORT_C,9),GPIO_OUT);

    
  int cur_time = 1000000 - 100000*msg_avail();

  while(1){
    

    // Включаем светодиод
  	gpio_set(GPIO_PIN(PORT_C,9));
    // Задача засыпает на 333333 микросекунды
   	xtimer_usleep(cur_time);
    // Выключаем светодиод
    gpio_clear(GPIO_PIN(PORT_C,9));
    // Задача снова засыпает на 333333 микросекунды
    xtimer_usleep(cur_time);
  }    
  return NULL;
}


int main(void)
{
  // Инициализация прерывания с кнопки (подробнее в примере 02button)
	gpio_init_int(GPIO_PIN(PORT_A,0),GPIO_IN,GPIO_BOTH,btn_handler,NULL);

  // Создение потоков (подробнее в примере 03threads)
  // Для первого потока мы сохраняем себе то, что возвращает функция thread_create,
  // чтобы потом пользоваться этим как идентификатором потока для отправки ему сообщений
  thread_one_pid = thread_create(thread_one_stack, sizeof(thread_one_stack),
                  THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  thread_one, NULL, "thread_one");
  // обратите внимание, что у потоков разный приоритет: на 1 и на 2 выше, чем у main
  thread_create(thread_two_stack, sizeof(thread_two_stack),
                  THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
                  thread_two, NULL, "thread_two");

  while (1){

    }

    return 0;
}

/* 
  Задание 1: Добавьте в код подавление дребезга кнопки
  Задание 2: Сделайте так, чтобы из прерывания по нажатию кнопки в поток thread_one передавалось целое число,
              которое означает, сколько раз должен моргнуть светодиод на пине PC8 после нажатия кнопки.
              После каждого нажатия циклически инкрементируйте значение от 1 до 5.
              Передать значение в сообщении можно через поле msg.content.value
  Задание 3: Сделайте так, чтобы из прерывания по отпусканию кнопки в поток thread_two передавалось целое число,
              которое задает значение интервала между морганиями светодиода на пине PC9. 
              Моргание светодиода не должно останавливаться.
              После каждого нажатия циклически декрементируйте значение от 1000000 до 100000 с шагом 100000.
              Чтобы послать сообщение асинхронно и без блокирования принимающего потока, нужно воспользоваться очередью. 
              Под очередь нужно выделить память в начале программы так: static msg_t rcv_queue[8];
              Затем в принимающем потоке нужно ее инициализировать так: msg_init_queue(rcv_queue, 8);
              Поток может проверять, лежит ли что-то в очереди, это делается функцией msg_avail(), 
              которая возвращает количество элементов в очереди. 
  Что поизучать: https://riot-os.org/api/group__core__msg.html
*/
