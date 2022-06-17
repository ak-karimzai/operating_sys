# How an interrupt are generated x86 architrue

1- вначале внешение устройство запршивает прыравание на своем порте 
2- отработчик переводит в векторном номере и запишет в CPU
3- отработчик поднимает соотвествуший прырование на cpu
4- отработчик прыравание ждет пока cpu знает прыравание и пока не принемает другие прыравание.
5- сpu признает и отработывает прырование.

**Обратим внимание, на то что PIC (programmable interrupt controller) или отработчик прырование не будет инициировать другое прерывание до тех пор, пока cpu не подтвердит текущее прерывание.**

```
Как только прерывание подтверждено ЦП, контроллер прерываний может запросить другое прерывание, независимо от того, закончил ли ЦП обработку предыдущего прерывания или нет. Таким образом, в зависимости от того, как ОС управляет ЦП, возможны вложенные прерывания.
```

Чтобы синхронизировать доступ к общим данным между обработчиком прерываний и другими потенциальными параллельными действиями, такими как инициализация драйвера или обработка данных драйвера, часто требуется включать и отключать прерывания контролируемым образом.

**на уровне устройства**

    путем программирования регистров управления устройством
**на уровне PIC (programmable interrupt controller)**

    PIC можно запрограммировать на отключение данной линии IRQ.
**на уровне процессора**

    например, на x86 можно использовать следующие инструкции:
    cli (снять флаг прерывания)
    sti (установить флаг прерывания)


# interrupt handling in Linux
В Linux обработка прерывания выполняется в три этапа: **критический**, **немедленный** и **отложенный**.

**На первом этапе**

     ядро запускает общий обработчик прерывания, который определяет номер прерывания, обработчик прерывания для этого конкретного прерывания и контроллер прерывания. В этот момент также будут выполняться любые критические по времени действия (например, подтверждение прерывания на уровне контроллера прерываний). Локальные прерывания процессора отключены на время этой фазы и продолжают отключаться в следующей фазе.

**На втором этапе** 
    
    будут выполнены все обработчики драйвера устройства, связанные с этим прерыванием. В конце этой фазы вызывается метод «конца прерывания» контроллера прерываний, чтобы позволить контроллеру прерываний подтвердить это прерывание. В этот момент разрешены прерывания локального процессора.

**Note**

    Возможно, что одно прерывание связано с несколькими устройствами, и в этом случае говорят, что прерывание является общим. Обычно при использовании общих прерываний драйвер устройства должен определить, является ли прерывание целевым для его устройства или нет.

# Tasklets
    
    Tasklets are a dynamic type (not limited to a fixed number) of deferred work running in interrupt context.

    Tasklets API:
        initialization: tasklet_init()
        activation: tasklet_schedule()
        masking: tasklet_disable(), tasklet_enable()
        
    Tasklets are implemented on top of two dedicated softirqs: TASKLET_SOFITIRQ and HI_SOFTIRQ

    Tasklets are also serialized, i.e. the same tasklet can only execute on one processor.

# Workqueues
    Workqueues are a type of deferred work that runs in process context.

    They are implemented on top of kernel threads.

    Workqueues API:
        init: INIT_WORK
        activation: schedule_work()

# Timers
    Timers are implemented on top of the TIMER_SOFTIRQ

    Timer API:
        initialization: setup_timer()
        activation: mod_timer()
# Deferrable actions summary
Here is a cheat sheet which summarizes Linux deferrable actions:

**tasklet** runs in interrupt context can be dynamically allocated same handler runs are serialized 

**workqueues** run in process context

**softIRQ** runs in interrupt context statically allocated same handler may run in parallel on multiple cores