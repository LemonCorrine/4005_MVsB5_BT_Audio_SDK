Example_PMU������Ҫ��ʾ��NVM �� powerkey�Ļ�������

1��Ӳ������Ҫ��:
	BP15ϵ�п����壬��Ӵ���С�壬TX/RX/GND ���� RX(A9)/TX(A10)/GND (������ 2000000)��

3�����ʹ��˵��:
	NVM ʾ������ʾ���״��ϵ�ͺ���Powerdown/PowerUP �ϵ��NVM���ݶ�ȡ���޸ģ�д��ǰ��ı仯���
	
	PowerKey ʾ��
	1) ��ʾ��ͨ��PowerKey���ƿ��������/�˳�Powerdown״̬�Ĳ���
	2) ��ʾ��ͨ��PowerKey������λ�����ƿ�����Ӳ����λ�Ĳ���
	
	�û������޸�SystemPowerKeyInit()����������������ģʽ���������ʱ�䡣
	����ģʽ��������Ҫ�����漸�֣�
	E_PWRKEY_MODE_BYPASS:       PowerKey���ܹر�
	E_PWRKEY_MODE_PUSHBUTTON:   ���PowerkeyΪ��������Ӧ�ã������趨ʱ��������/�˳�Powerdown״̬����ģʽ�г���8sӲ����λ�Ĺ���
	E_PWRKEY_MODE_SLIDESWITCH:  ���PowerkeyΪ�ζ���������Ӧ�ã����ذε��͵�ƽ����Powerdown���ߵ�ƽPowerUp����ģʽ���޳���8sӲ����λ����
	
	