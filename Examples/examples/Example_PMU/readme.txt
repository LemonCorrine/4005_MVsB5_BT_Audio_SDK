Example_PMU������Ҫ��ʾ��NVM �� powerkey�Ļ�������

1��Ӳ������Ҫ��:
	BP15ϵ�п����壬��Ӵ���С�壬TX/RX/GND ���� RX(A9)/TX(A10)/GND (������ 2000000)��

2�����ʹ��˵��:
	NVM ʾ������ʾ���״��ϵ�ͺ���Powerdown/PowerUP �ϵ��NVM���ݶ�ȡ���޸ģ�д��ǰ��ı仯���
	
	PowerKey ʾ��
	1) ��ʾ��ͨ��PowerKey���ƿ��������/�˳�Powerdown״̬�Ĳ���
	2) ��ʾ��ͨ��PowerKey������λ�����ƿ�����Ӳ����λ�Ĳ���(��Ҫ�򿪺궨�� DEMO_8S_RESET)��
	
	�û������޸�SystemPowerKeyInit()����������������ģʽ���������ʱ�䡣
	����ģʽ��������Ҫ�����漸�֣�
	E_PWRKEY_MODE_BYPASS:           PowerKey���ܹر�
	E_PWRKEY_MODE_PUSHBUTTON:       ���PowerkeyΪ��������Ӧ�ã������趨ʱ��������/�˳�Powerdown״̬����ģʽ�г���8sӲ����λ�Ĺ���
	E_PWRKEY_MODE_SLIDESWITCH_HON:  ���PowerkeyΪ�ζ���������Ӧ�ã����ذε��͵�ƽ����Powerdown���ߵ�ƽPowerUp����ģʽ���޳���8sӲ����λ����
	E_PWRKEY_MODE_SLIDESWITCH_LON:  ���PowerkeyΪ�ζ���������Ӧ�ã����ذε��ߵ�ƽ����Powerdown���͵�ƽPowerUp����ģʽ���޳���8sӲ����λ����
	
3������ע�����
   1) ����Ķ����º󣬿�����Ӧ�Ͽ����й���(�������������磬USB�����)�������ϵ���µ��޸����ò�����Ч
   2) ��������뿪�����ã���BP15XX_DEVELOPMENT_V1.3�������ϣ�P2�ε�ONλ��ʱ��PowerkeyΪ�͵�ƽ��P2�ε�OFFλ��ʱ��PowerkeyΪ�ߵ�ƽ��
                          ����E_PWRKEY_MODE_PUSHBUTTONӦ��ʱ��P2Ӧ�ε�OFFλ�ã�