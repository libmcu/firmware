===
PMU
===

BQ25180
=======
.. uml::
    :caption: A Class Diagram

    interface i2c_interface {}
    interface INTCallback {}
    class BQ25180 {
        reset()
    }
    Controller --> BQ25180
    Controller --> i2c_interface
    Controller ..|> INTCallback
    Controller ..|> BQ25180_IO
    i2c_impl ..|> i2c_interface
    BQ25180 --> INTCallback
    BQ25180 --> BQ25180_IO

기능
----

배터리 보조 모드
~~~~~~~~~~~~~~~~

충전전류가 0으로 떨어지고 시스템 로드가 입력 전류 제한 이상일 때, *SYS* 전압은 더 떨어집니다. *SYS* 전압이 $V_{BSUP1}$ 보다 더 떨어질 경우, 배터리가 시스템 로드를 보조합니다. *SYS* 전압이 $V_{BSUP2}$ 이상 높아지면 배터리 보조는 중지됩니다. 배터리 보조 모드일 때 전류는 regulated 되지 않습니다. 하지만, *BATOCP* 보호회로는 활성화시킨 경우 동작합니다. 배터리 보조 모드가 동작하기 위해서 배터리 전압은 battery undervoltage lockout threshold (*VBUVLO*) 전압보다 높아야 합니다.

SYS 출력 제어
~~~~~~~~~~~~~

* 전원
* regulation

입력 전류 제어
~~~~~~~~~~~~~~

보호 장치
~~~~~~~~~

버튼
~~~~

와치독
~~~~~~

* 전원인입 후 15초 와치독

리셋
~~~~

* 하드웨어 리셋
* 소프트웨어 리셋

인터럽트
~~~~~~~~
* 인터럽트 설정 및 콜백

온도 측정
~~~~~~~~~
* NTC 기능

.. doxygenfile:: drivers/include/drivers/bq25180.h
   :project: fpl
