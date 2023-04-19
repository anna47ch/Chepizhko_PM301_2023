// Подключение библиотек
#include "xtimer.h"
#include "timex.h"
#include "periph/gpio.h"

#define INTERVAL 200000

uint32_t time = 0;

void btn_handler(void *arg)
{
  (void)arg;

  gpio_read(GPIO_PIN(PORT_C,8));


  if( xtimer_now_usec() >= time + 100000)
  {

    gpio_toggle(GPIO_PIN(PORT_C, 8));
    time = xtimer_now_usec();
  }
}


int main(void)
{

  
  gpio_init_int(GPIO_PIN(PORT_A, 0), GPIO_IN, GPIO_BOTH, btn_handler, NULL);
 
	gpio_init(GPIO_PIN(PORT_C, 8), GPIO_OUT);

  while(1){
    
    
    
  }
  return 0;
}

/*
Задание 1. Подавите дребезг контакта кнопки. Для этого запомните время, когда вызвалось прерывание, 
            и при последующем вызове, если не прошло 100 мс, не переключайте светодиод.
Задание 2. Сделайте так, чтобы при удержании кнопки светодиод светился, а при отпускании - гас. 
            В этом случае нужно заставить прерывание срабатывать и на фронт, и на спад. 
            Это задается при инициализации прерывания макросом GPIO_BOTH. 
            Обработчик прерывания должен узнать, его вызвал фронт или спад.
            Для этого можно прочитать значение пина с кнопкой функцией gpio_read. 
            Если прочитали высокое состояние - то это был фронт, если низкое - это был спад.   
*/
