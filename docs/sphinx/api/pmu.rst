===
PMU
===

BQ25180
=======
.. uml::
    :caption: A Class Diagram

    interface i2c_interface {}
    interface INTCallback {}
    interface BQ25180_IO {
        read()
        write()
    }
    class BQ25180 {
        reset(soft or hard)
        enable_battery_charging(bool enable)
        set_tricklecharge_threshold(voltage)
        set_precharge_threshold(voltage)
        set_fastcharge_threshold(voltage)
        set_tricklecharge_current()
        set_precharge_current()
        set_fastcharge_current()
        set_battery_regulation_voltage()
        set_termination_current()
        enable_vindpm(bool enable)
        set_vindpm_voltage()
        enable_dppm(bool enable)
        set_battery_discharge_current()
        set_input_current()
        set_sys_voltage()
        set_sys_mode()
    }
    Controller --> BQ25180
    Controller --> i2c_interface
    Controller ..|> INTCallback
    Controller ..|> BQ25180_IO
    i2c_impl ..|> i2c_interface
    BQ25180 --> INTCallback
    BQ25180 --> BQ25180_IO

충전 흐름
---------

1. 배터리 전압( $V_{BAT}$ )이 $V_{BATSC}$ [#f1]_ 보다 낮은 경우 trickle 충전
2. 배터리 전압이 $V_{BATSC}$ [#f1]_ 보다 높고, $V_{LOWV}$ [#f2]_ 보다 낮은 경우 precharge
3. $V_{LOWV}$ [#f2]_ 보다 높은 경우 fast 충전
4. $I_{TERM}$ [#f3]_ 에 도달한 경우 충전 종료

배터리 데미지를 피하기 위해 배터리 전압이 $V_{BATSC}$ [#f1]_ 보다 낮은 경우, $I_{BATSC}$ [#f4]_ 전류로 충전합니다(trickle 충전).

$V_{BATSC} < V_{BAT} < V_{LOWV}$ 의 경우, $I_{PRECHG}$ [#f5]_ 전류로 충전합니다(precharge).

배터리 전압이 $V_{LOWV}$ 에 다다르면, `ICHG_CTRL.ICHG` 전류로 충전합니다. 배터리 전압이 *VBATREG* [#f6]_ 에 다다르면, 전류를 줄이기 시작합니다. 충전 전류가 $I_{TERM}$ [#f3]_ 에 다다르면 충전이 종료됩니다.

.. [#f1] $V_{BATSC} = 1.8V$
.. [#f2] $V_{LOWV}$ = `IC_CTRL.VLOWV_SEL` 설정에 따라 3V 이거나 2.8V
.. [#f3] $I_{TERM}$ = `CHARGECTRL0.ITERM` 설정에 따라 ICHG의 5%, 10%, 20% 또는 사용안함
.. [#f4] $I_{BATSC} = 8mA$
.. [#f5] $I_{PRECHG}$ = `CHARGECTRL0.IPRECHG` 설정에 따라 I_{TERM} 의 2배이거나 1배
.. [#f6] $VBATREG$ = 배터리 regulation 전압. 최대 4.65V 로 `VBAT_CTRL.VBATREG` 에서 설정

기능
----

입력전압 기반 전원 관리
~~~~~~~~~~~~~~~~~~~~~~~

입력전압 기반 전원 관리 기능은 충전전류를 조정하여 입력전압 강하가 일어나지 않도록 방지합니다. 기능이 enabled 되어 있다면, 입력전압이 항상 `CHARGECTRL0.VINDPM` 보다 높게 유지되도록 하고, 전원관리 활성화 여부를 `STAT0.VINDPM_ACTIVE_STAT` 플래그에 설정합니다.

.. caution:: 입력전압 기반 전원 관리가 활성화된 경우, `CHARGECTRL0.ITERM` 에 설정한 배터리 종료 기능은 동작하지 않습니다. 

실시간 전원 경로 관리
~~~~~~~~~~~~~~~~~~~~~

유효한 전원 소스가 연결되면 지속적으로 입력 전압과 전류를 감시합니다. *IN* 으로 인입된 전류는 배터리 충전과 시스템 전원에 사용되고 *SYS* 핀에 공급됩니다.

실시간 전원 경로 관리 기능은 입력 최대전류인 1.1A 를 초과해 사용할 경우 입력전류를 그 이하로 제한합니다. 또한,  *SYS* 전압이 $V_{DPPM}$ [#f7]_ 보다 낮을 경우, 배터리 충전전류를 낮춥니다. *SYS* 전압은 배터리 전압보다 높게 유지됩니다.

`SYS_REG.VDPPM_DIS` 로 이 기능을 비활성화하면 낮은 VSYS 에도 충전을 진행할 수 있습니다.

.. caution:: 실시간 전원 경로 관리 기능이 활성화된 경우, `CHARGECTRL0.ITERM` 에 설정한 배터리 종료 기능은 동작하지 않습니다. 

.. [#f7] $V_{DPPM} = V_{BAT} + 100mV$

배터리 보조 모드
~~~~~~~~~~~~~~~~

충전전류가 0으로 떨어지고 시스템 로드가 입력 전류 제한(`TMR_ILIM.ILIM`) 이상일 때, *SYS* 전압은 더 떨어집니다. 이때, *SYS* 전압이 $V_{BSUP1}$ [#f8]_ 보다 더 떨어질 경우, 배터리가 시스템 로드를 보조합니다. 반대로 *SYS* 전압이 $V_{BSUP2}$ [#f9]_ 이상 높아지면 배터리 보조는 중지됩니다.

배터리 보조 모드일 때 전류는 regulated 되지 않습니다. *BATOCP* (`CHARGECTRL1.IBAT_OCP`) 가 활성화되어 있는 경우 과전류 보호회로는 동작합니다.

배터리 보조 모드가 동작하기 위해서 배터리 전압은 battery undervoltage lockout threshold (`CHARGECTRL1.BUVLO`) 전압보다 높아야 합니다.

.. caution:: 배터리 보조 모드가 활성화된 경우, `CHARGECTRL0.ITERM` 에 설정한 배터리 종료 기능은 동작하지 않습니다. 

.. [#f8] $V_{BUSP1} = V_{BAT} - 40mV$
.. [#f9] $V_{BUSP2} = V_{BAT} - 20mV$

SYS 제어
~~~~~~~~

`SYS_REG.SYS_MODE` 설정으로 *SYS* 모드를 제어할 수 있습니다.

========  =========  ========================== =================================
SYS_MODE  Supply     PullDown                   Desc.
========  =========  ========================== =================================
00        IN or BAT  Off except during HW reset Normal Operation
01        BAT        Off except during HW reset Force BAT power (IN disconnected)
10        None       Off                        SYS Off –Floating
11        None       On                         SYS Off – Pulled Down
========  =========  ========================== =================================

배터리 충전중에 *Force BAT power* 모드로 전환하는 경우, 충전은 중지되고 *SYS* 는 배터리로부터 전원을 공급받습니다.

배터리 전압이 $V_{BUVLO}$ 보다 낮을 경우 설정에 관계없이 *Normal Operation* 모드로 동작합니다.

*SYS* 출력 전압은 3 가지 방식이 있습니다:

1. Battery Tracking Mode
2. Fixed Voltage Mode
3. Pass Through Mode

*Batter Tracking Mode* 는$V_{BAT}$ + 225 mV 를 유지하고, *Fixed Voltage Mode* 는 설정한 전압을 유지합니다. 마지막으로 *Pass Through Mode* 는 regulation 없이 $V_{IN}$ 을 그대로 출력합니다.

입력 전류 제어
~~~~~~~~~~~~~~

입력 전류 제한을 설정할 수 있습니다.

보호 장치
~~~~~~~~~

버튼
~~~~

와치독
~~~~~~

리셋
~~~~

인터럽트
~~~~~~~~

온도 모니터
~~~~~~~~~~~

API
---

.. doxygenfile:: drivers/bq25180.h
   :project: fpl
