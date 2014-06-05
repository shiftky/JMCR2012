//==========================================================================//
//  MCR2009 �V��v���O���� �C����                                         //
//                                                                          //
//  4�֋쓮�EEEPROM���s���O�ۑ��Ή�                                         //
//  LCD���s���[�h�\���̃o�O���C���ς�(2011.01.18)                           //
//  �p�����[�^�ݒ�EEP-ROM�ւ̕ۑ��֑Ή�(2011.03.28)                        //
//  �⓹���s��X�s�[�h����̋@�\�ǉ�/�C��(2011.04.09)                        //
//�@�W���p���}�C�R���J�[�����[2012 �k�M�z���o��p(2011.11.26)             //
//==========================================================================//
#include "3048f.h"
#include <machine.h>
#include "Param_Eeprom.h"      //EEP-ROM�����ݒ� �p�����[�^�ݒ�ۑ��Ή���
#include "wait.h"          //�x���v���O����
#include "lcd.h"          //LCD�\���v���O����
#include "sci.h"          //I2C�ʐM�d�l�w�b�_�t�@�C��

//==========================================================================//
//  ��`��                                                                  //
//==========================================================================//
#define    SV_TURN    (PB.DR.BIT.B6)        //�T�[�{��]�����@�E0 ��1
#define    BLV_A    (PB.DR.BIT.B2 = 1)      //����� ���]
#define    BLV_B    (PB.DR.BIT.B2 = 0)      //����� �t�]
#define    BRV_A    (PB.DR.BIT.B3 = 0)      //�E��� ���]
#define    BRV_B    (PB.DR.BIT.B3 = 1)      //�E��� �t�]
#define    FLV_A    (PA.DR.BIT.B1 = 1)      //���O�� ���]
#define    FLV_B    (PA.DR.BIT.B1 = 0)      //���O�� �t�]
#define    FRV_A    (PA.DR.BIT.B3 = 1)      //�E�O�� ���]
#define    FRV_B    (PA.DR.BIT.B3 = 0)      //�E�O�� �t�]

#define    BZ      (PB.DR.BIT.B7)           //�u�U�[

#define    P_SW1    ((~PA.DR.BIT.B5) & 0x01)  //�v�b�V���X�C�b�`1
#define    P_SW2    ((~PA.DR.BIT.B6) & 0x01)  //�v�b�V���X�C�b�`2
#define    P_SW3    ((~PA.DR.BIT.B7) & 0x01)  //�v�b�V���X�C�b�`3
#define    P_SW4    ((~PB.DR.BIT.B0) & 0x01)  //�v�b�V���X�C�b�`4

#define    AS_L    (( AD.DRA >> 8) & 0xFF)    //�A�i���O��
#define    AS_R    (( AD.DRB >> 8) & 0xFF)    //�A�i���O�E
#define    PO_S    (( AD.DRC >> 8) & 0xFF)    //�|�e���V���Z���T
#define    AS_SAKA    (( AD.DRD >> 8) & 0xFF)   //�⌟�o�A�i���O�Z���T
#define    DS_SAKA    ((~P8.DR.BIT.B4) & 0x01)  //�⌟�o�f�W�^���Z���T

#define    DS_LO    ((~P7.DR.BIT.B7) & 0x01)  //�f�W�^�����O
#define    DS_LI    ((~P7.DR.BIT.B6) & 0x01)  //�f�W�^������
#define    DS_RI    ((~P7.DR.BIT.B5) & 0x01)  //�f�W�^���E��
#define    DS_RO    ((~P7.DR.BIT.B4) & 0x01)  //�f�W�^���E�O
#define    DS_C    ((~P3.DR.BIT.B0) & 0x01)   //�f�W�^������
#define    DS_S    ((~P3.DR.BIT.B1) & 0x01)   //�f�W�^����ǂ�

#define    S_SW    ((~P3.DR.BIT.B2) & 0x01)  //�X�^�[�g�Z���T
#define    DIP1    ((~P3.DR.BIT.B3) & 0x01)  //DIP�X�C�b�`1
#define    DIP2    ((~P3.DR.BIT.B4) & 0x01)  //DIP�X�C�b�`2
#define    DIP3    ((~P3.DR.BIT.B5) & 0x01)  //DIP�X�C�b�`3
#define    DIP4    ((~P3.DR.BIT.B6) & 0x01)  //DIP�X�C�b�`4
#define    DIP_ALL    (((~P3.DR.BYTE) & 0x78)>>3)  //DIP�X�C�b�`2�i�p
#define    D_SW    ((P6.DR.BYTE) & 0x0f)    //CPU��� DIP�X�C�b�`
#define    D_SW1    ((~P6.DR.BIT.B0) & 0x01)

#define    LED7    (P4.DR.BIT.B0)        //LED7
#define    LED8    (P4.DR.BIT.B1)        //LED8
#define    LED9    (P4.DR.BIT.B2)        //LED9
#define    LED10   (P4.DR.BIT.B3)        //LED10
#define    LED11   (P4.DR.BIT.B4)        //LED11
#define    LED12   (P4.DR.BIT.B5)        //LED12
#define    LED13   (P4.DR.BIT.B6)        //LED13
#define    LED14   (P4.DR.BIT.B7)        //LED14

#define    SENSOR    ((P7.DR.BYTE>>2) & 0x3c) + (P3.DR.BYTE & 0x03)

#define    SV_B    (P8.DR.BIT.B4)
#define    RL_B    (P8.DR.BIT.B3)
#define    RR_B    (P8.DR.BIT.B2)
#define    FL_B    (P8.DR.BIT.B1)
#define    FR_B    (P8.DR.BIT.B0)

//==========================================================================//
//  �萔                                                                    //
//==========================================================================//
#define    PWM_CYCLE    3071    //���[�^PWM�̃T�C�N�� 1ms  1536 1843 2150 2458 2765


#define   on     1        //ON
#define   off    0        //OFF

#define   L      1        //R�n���h�����O
#define   R      0        //L�n���h�����O

#define   F      1        //�O��
#define   B      0        //���

#define   FREE    1        //���[�^�[���[�h�t���[
#define   BRAKE   0        //���[�^�[���[�h�u���[�L

#define   H_SV_KP    220
#define   H_SV_KD    2000
#define   H_TR_KP    30
#define   H_TR_KD    180

//==========================================================================//
//  �֐��v���g�^�C�v�錾                                                    //
//==========================================================================//
void ini();              //������

void motor_mode_s(int);                //���[�^�[���[�h �T�[�{
void motor_mode_f(int, int);           //���[�^�[���[�h �O��
void motor_mode_r(int, int);           //���[�^�[���[�h ���
void speed_f(int, int);                //�O�փ��[�^�[
void speed_r(int, int);                //��փ��[�^�[
void sp_enc_f(unsigned char, unsigned char, int);  //�O�փ��[�^�[
void sp_enc_r(unsigned char, unsigned char, int);  //��փ��[�^�[
void enc_f(unsigned char, int);        //�O�փ��[�^�[
void enc_r(unsigned char, int);        //��փ��[�^�[
int diff(int, int);                    //����PWM�v�Z

void PD_Trace(void);              //�����g���[�X PD����
void pd_angle(unsigned char);     //PD angle module
int get_AS(void);                 //�A�i���O�Z���T�l�擾
void servo_pwm(int, int);         //�T�[�{PWM����

void lcd_data(void);              //LCD�\��
void run_data(void);              //EEPROM�f�[�^ PC�o��
void sci_monitor(void);           //SCI���j�^�����O

void param_set(void);             //�p�����[�^�l �ݒ�
void param_load(void);            //�p�����[�^�l ���s�O�ǂݍ���
void param_init(void);            //�p�����[�^�l ���s�O�ϐ�������
void param_check(void);           //�p�����[�^�l �ꊇPC���M

void stop(void);                  //��~����
void error(int, int, int, int);   //��~����ON/OFF

void cl1(void);                  //�J�E���g�N���A1
void cl2(void);                  //�J�E���g�N���A2
void cl3(void);                  //�J�E���g�N���A3
void cl4(void);                  //�J�E���g�N���A4
void cl5(void);                  //�J�E���g�N���A5
void cl6(void);                  //�J�E���g�N���A6
void cl7(void);                  //�J�E���g�N���A7
void cl8(void);                  //�J�E���g�N���A8
void cl9(void);                  //�J�E���g�N���A9
void all_cl(void);               //ALL �J�E���g�N���A

//==========================================================================//
//  ���ϐ��錾                                                            //
//==========================================================================//
//=====���샂�[�h�ELCD�\�����[�h�֘A=====//
int mod, lcd_modeview, lcdmode, sen;

//=====�Z���T�[�֘A=====//
unsigned int sensor;

//=====���[�^�[�쓮�֘A=====//
int mov;

//=====�|�e���V���֘A=====//
int ang1, ang2, ang3;
unsigned char pot;
signed int h_before;

//=====���[�^���[�G���R�[�_�֘A=====//
char counter;
long now_enc, enc_buff, enc_def, en_cn, dis_cnt, stop_cnt, normal_enc_cnt;
unsigned int enc_cnt, enc;
unsigned long tim_s, tim_s1;

//=====PD_Trace()�֘A=====//
signed int v_before;
int tr_f, v;
int sensor_pattern, dg_tr_f;

//=====EEP-ROM�֘A=====//
int addr;
char m_dat[8];
char cnt_write, eep_set;
char f_diff, b_diff, angle, fl_mo, fr_mo, bl_mo, br_mo, Digital;
unsigned char save_f;

//=====��~�����֘A=====//
int error_cnt1, error_cnt2, error_cnt3, error_cnt4;
char er1, er2, er3, er4;
unsigned long stp_cnt, stop_dis;
int error_num;

//=====�⓹�֘A=====//
int sak_as_cnt;
int sak_ds_cnt;

//=====�ėp�J�E���^�E�X�g�b�v�p�J�E���^=====//
unsigned long cnt1, cnt2, cnt3, cnt4, cnt5, cnt6, cnt7, cnt8, cnt9;
unsigned long cnt_stop;
unsigned int ck_cnt;

//=====�p�����[�^�ݒ�p=====//
int select_item;
char parameter[16];

/*0*/char eep_enc_speed;        //�G���R�[�_���� �ʏ푖�s���X�s�[�h

/*1*/char eep_curve_angle;        //�G���R�[�_���� �ʏ푖�s �J�[�u����p�x
/*2*/char eep_curve_speed;        //�G���R�[�_���� �ʏ푖�s �J�[�u���s���X�s�[�h

/*3*/char eep_crank_inspeed;      //�N�����N �i���X�s�[�h
/*4*/char eep_crank_angle;        //�N�����N �T�[�{PWM��

/*5*/char eep_lane_inspeed;        //���[���`�F���W �i���X�s�[�h
/*6*/char eep_l_lane1;          //���[���`�F���W ���؂�p
/*7*/char eep_l_lane2;          //���[���`�F���W ���؂�Ԃ��p
/*8*/char eep_r_lane1;          //���[���`�F���W �E�؂�p
/*9*/char eep_r_lane2;          //���[���`�F���W �E�؂�Ԃ��p

/*10*/char eep_r_diff;          //�⓹���s �o��X�s�[�h
/*11*/char eep_enc_topbrake;      //�Ⓒ���s �u���[�L
/*12*/char eep_tr_kp;          //PD_Trace KP
/*13*/char eep_tr_kd;          //PD_Trace KD
/*14*/char eep_sak_sensor;        //�⓹�A�i���O�Z���T�[ �⌟�o臒l

/*15*/char eep_run_dis;          //�����s�����i�����X�g�b�v�p�j

//==========================================================================//
//�@�e�퐧��p�z��                                                          //
//==========================================================================//
// ���[�^���[�G���R�[�_���x����֘A
// 2011.09.30 �쐬   200ppr�~2�i���G�b�W�J�E���g�j
// �v���^�C��D = 33.5mm
// �z��v�f �~�i���x�~10�j���w�葬�x�p���X��
// ��F2.5m/s�̃p���X���@enc_speed_pulse[25]��25�Ԗڂ̔z��v�f
// �Ή��� 7 m/s �܂�
const int enc_speed_pulse[] = {
  0, 3, 7, 11, 15, 19, 22, 26, 30, 34,
  38, 41, 45, 49, 53, 57, 60, 64, 68, 72,
  76, 80, 83, 87, 91, 95, 99, 102, 106, 110,
  114, 118, 121, 125, 129, 133, 137, 140, 144, 148,
  152, 156, 160, 163, 167, 171, 175, 179, 182, 186,
  190, 194, 198, 201, 205, 209, 213, 217, 220, 224,
  228, 232, 236, 240, 243, 247, 251, 255, 259, 262,
  266, 270, 274, 278, 281, 285, 289, 293, 297, 300,
  304
};

// ���֍��v�Z�p
// �O�� W = 0.16  T = 0.15
const f_revolution_difference[] = {
    100, 98, 97, 95, 94,
    92, 91, 89, 88, 86,
    85, 83, 82, 80, 79,
    78, 76, 75, 74, 72,
    71, 70, 68, 67, 65,
    64, 63, 61, 60, 59,
    57, 56, 55, 53, 52,
    51, 49, 48, 46, 45,
    44, 42, 41, 39, 38,
    36
};

// ��� W = 0.16  T = 0.17
const b_revolution_difference[] = {
    100, 98, 96, 95, 93,
    91, 89, 88, 86, 84,
    83, 81, 80, 78, 77,
    75, 74, 72, 71, 69,
    68, 66, 65, 63, 62,
    60, 59, 57, 56, 55,
    53, 52, 50, 49, 47,
    46, 44, 43, 41, 40,
    38, 37, 35, 34, 32,
    31
};

//==========================================================================//
//  ������                                                                  //
//==========================================================================//
void ini(void)
{
  //=====�ϐ�������=====//
  cnt1 = 0;
  cnt2 = 0;
  cnt3 = 0;
  cnt4 = 0;
  cnt5 = 0;
  cnt6 = 0;
  cnt7 = 0;
  cnt8 = 0;
  cnt9 = 0;

  stop_cnt = 0;
  stp_cnt = 0;

  sensor = 0;
  sensor_pattern = 0;

  mov = 1;

  mod = 1;
  lcdmode = 0;
  lcd_modeview = 0;
  eep_set = 0;
  save_f = off;          //EEP�t���O
  addr = 16;

  dg_tr_f = 1;

  er1 = 0;
  er2 = 0;
  er3 = 0;
  er4 = 0;

  error_cnt1 = 0;
  error_cnt2 = 0;
  error_cnt3 = 0;
  error_cnt4 = 0;
  error_num = 0;

  sak_as_cnt = 0;
  sak_ds_cnt = 0;

  dis_cnt = 0;
  en_cn = 0;
  now_enc = 0;
  enc_buff = 0;
  enc_def = 0;

  tr_f = 1;

  //=====�p�����[�^�ݒ�p�ϐ�������=====//
  select_item = 0;

  eep_enc_speed = 40;

  eep_curve_angle = 5;
  eep_curve_speed = 38;

  eep_crank_inspeed = 16;
  eep_crank_angle  = 95;

  eep_lane_inspeed = 25;
  eep_l_lane1 = 21;
  eep_l_lane2 = 22;
  eep_r_lane1 = 28;
  eep_r_lane2 = 22;

  eep_r_diff = 1;
  eep_enc_topbrake = 28;
//  eep_tr_kp = 4;
//  eep_tr_kd = 18;
  eep_sak_sensor = 17;

  eep_run_dis = 70;

  //=====H8������=====//
  //���o�͐ݒ�@0�cInput�@1�cOutput
  P3.DDR = 0x00;
  P4.DDR = 0xff;
  P6.DDR = 0xf0;          //CPU��DIP_SW
  P8.DDR = 0x1f;
  P9.DDR = 0x01;
  PA.DDR = 0x1e;
  PB.DDR = 0xfe;
  
//  P3.DR.BYTE = 0x00;
//  P4.DR.BYTE = 0x00;        //LED ALLOFF
//  P9.DR.BYTE = 0x01;
//  PA.DR.BYTE = 0x00;
//  PB.DR.BYTE = 0x32;

  //=====A/D initialize=====//
  AD.CSR.BYTE = 0x3b;        //AD�ϊ��������ݒ� AD�ϊ��X�^�[�g
  AD.CR.BYTE = 0x00;        //AD�R���g���[�����W�X�^�@�O���g���K���͂ɂ��AD�ϊ��̊J�n�̋��E�֎~�@�ʏ��ALL0

  //=====ITU0 ���O���[�^�ݒ�=====//
  ITU0.TCR.BYTE = 0x23;
  ITU0.GRA = PWM_CYCLE;
  ITU0.GRB = 0; 
  ITU0.TIER.BYTE = 0x01;

  //=====ITU1 �E�O���[�^�ݒ�=====//
  ITU1.TCR.BYTE = 0x23;
  ITU1.GRA = PWM_CYCLE;
  ITU1.GRB = 0;

  //=====ITU2 ���[�^���[�G���R�[�_ ���͐ݒ�=====//
  ITU2.TCR.BYTE = 0x04;      //200pulse

  //=====ITU3,4 ��֍��E�E�T�[�{���[�^�ݒ�=====//
  ITU3.TCR.BYTE = 0x22;
  ITU.TFCR.BYTE = 0x3e;
  ITU3.GRA = PWM_CYCLE;
  ITU3.GRB = ITU3.BRB = 0;
  ITU4.GRA = ITU4.BRA = 0;
  ITU4.GRB = ITU4.BRB = 0;
  ITU.TOER.BYTE = 0x38;

  ITU.TMDR.BYTE = 0x1b;      //PWM���[�h�I�� 03 ? 1b
  ITU.TSTR.BYTE = 0x0f;      //ITU�J�E���g�X�^�[�g 1f

  //=====LCD������=====//
  P4.DR.BYTE = 0x00;        //������
  init_lcd();

  set_ccr( 0x00 );          //�S�̊��荞�݋���

  ang1 = PO_S;              //�p�x0�x�w��
}

//==========================================================================//
//  ���C���v���O����                                                        //
//==========================================================================//
void main(void)
{
  int ck = 0, crank = 0;

  //=====������=====//
  ini();                    //CPU��ϐ�������
  initI2CEeprom(&P9DDR, &P9DR, 0x01, 4, 2);
  init_sci(0x00, 19);       //SCI������

  //=====�}�C�R���J�[�̏�ԏ�����=====//
  motor_mode_f( FREE, FREE );
  motor_mode_r( FREE, FREE );
  motor_mode_s( FREE );
  dg_tr_f = 1;

  //=====CPU�������݃X�C�b�`�ؑփ~�X�h�~=====//
  BZ = 1;
  wait10(15);
  BZ = 0;

  //=====�X�^�[�g����=====//
  /*
  DIP 0  �f�[�^�ۑ������@�ʏ푖�s
  DIP 1  EEP-ROM�f�[�^�����@�f�[�^�ۑ��L��
  DIP 2  ����m�F�i���[�^�[OFF�j
  DIP 1,2  ���s�f�[�^PC�o��
  DIP 3  LCD�f�[�^�\��
  DIP 1,3  SCI���j�^�����O�iPC�f�[�^�\���j
  */
  switch(DIP_ALL){
    //�f�[�^�ۑ��Ȃ� �ʏ푖�s
    case 0:
    case 8:
      if(P_SW1){goto LABEL2;}
      if(P_SW2){goto LABEL1;}
      if(P_SW3){goto LABEL4;}
      if(P_SW4){goto LABEL3;}
      mov = 1;
      mod = 1;
      break;

    //EEP-ROM�f�[�^���� �f�[�^�ۑ�����
    case 1:
    case 9:
      LABEL1:          //GOTO���x��
      lcd_write(1, "Data Cleaning...");
      Param_clearI2CEeprom();    //3�`4�b�ҋ@
      lcd_write(2, "Compleart!!");
      eep_set = 1;
      wait10(500);
      mov = 1;
      break;

    //����m�F�i���[�^�[OFF�j
    case 2:
    case 10:
      LABEL2:          //GOTO���x��
      mov = 0;
      mod = 10;
      lcd_modeview = 1;
      lcd_write(1,"Motor OFF Mode");
      wait10(500);
      break;

    //���s�f�[�^PC���M
    case 3:
    case 11:
      LABEL3:          //GOTO���x��
      tr_f = 0;
      run_data();
      break;

    //LCD�f�[�^�\��
    case 4:
    case 12:
      LABEL4:          //GOTO���x��
      tr_f = 1;
      lcd_data();
      break;

    //SCI���j�^�����O
    case 5:
    case 13:
      tr_f = 0;
      sci_monitor();
      break;
  }

  //=====���s�X�^�[�g�ҋ@=====//
  START:
  lcd_clear(0);          //LCD ALL clear
  lcd_write(1,"Ready...   BAR:");
  lcd_write(2,"P_SW1 Start");

  while(!P_SW1){
    tr_f = 1;
    all_cl();
    lcd_cursor(1, 16);
    lcd_put_dig(S_SW, 1);

    //=====���s�p�����[�^�ݒ��ʂ�=====//
    if(P_SW3){
      lcd_clear(0);
      lcdmode = 0;
      param_set();
      goto START;
    }

    //=====���s�p�����[�^�`�F�b�N��ʂ�=====//
    if(P_SW4 && !DIP3){
      lcd_clear(0);
      lcdmode = 0;
      param_load();
      BZ = 1; wait10(100); BZ = 0; wait10(50);
      BZ = 1; wait10(100); BZ = 0; wait10(50);
      goto START;
    }
    else if(P_SW4 && DIP3){
      BZ = 1; wait10(150); BZ = 0; wait10(50);
      lcd_clear(0);
      lcdmode = 0;
      param_check();
    }

    //=====�X�^�[�g�J�E���g�^�C�}�[====//
    if(P_SW2){
      lcd_clear(0);
      lcd_write(1,"Start Timer");
      lcd_home(2);
      lcd_print(" 3"); BZ=1; wait10(400); BZ=0; wait10(400);
      lcd_print(" 2"); BZ=1; wait10(400); BZ=0; wait10(400);
      lcd_print(" 1"); BZ=1; wait10(400); BZ=0; wait10(400);
      break;
    }
  }

  lcd_clear(0);
  param_init();

  pot = PO_S;
  stop_cnt = 0;

  //=====�X�^�[�g�o�[�Z���T�[�ɂ���ĊJ�n=====//
  while(S_SW){
    all_cl();
    lcd_write(1,"S_SW Wait");
  }

  all_cl();
  lcd_clear(0);

  //=====�T�[�{����e�X�g�p�i�ʏ�R�����g�A�E�g�j=====//
  /*
  while(1){
    //pot_angle(124, R);
    //pd_angle( pot );
    servo_pwm( eep_crank_angle , R);
  }
  */

  //=====pd_angle�e�X�g�p�i�ʏ�R�����g�A�E�g�j=====//
  /*
  while(1){
    lcd_home(1);
    lcd_print(" H_SV_KP:");
    lcd_put_dig(H_SV_KP, 6);
    lcd_home(2);
    lcd_print(" H_SV_KD:");
    lcd_put_dig(H_SV_KD, 6);

    if( cnt8 < 1500 ){
      pd_angle( pot + eep_l_lane1 );
    }
    else{
      pd_angle( pot - eep_l_lane2 );
      if( P_SW1 ){
        cnt8 = 0;
      }
    }
  }
  */

  //=====�N�����N����e�X�g�p�i�ʏ�R�����g�A�E�g�j=====//
  /*
  dis_cnt = 0;
  while(1){
    if( dis_cnt < 200 ){
      sp_enc_f(5,5);
      sp_enc_r(5,5);
    }
    else if( dis_cnt > 200 ){
      pd_angle( pot - eep_crank_angle );
      speed_f( -90, 95, off );
      speed_r( 25, 55, off );
    }
  }
  */

  //=====���C���v���O����=====//
  while(1){
    //=====EEP-ROM�f�[�^�ۑ�=====//
    if( eep_set == 1 && lcd_modeview == 0 ){
      save_f = 1;        //�f�[�^�ۑ��̊J�n
      I2CEepromProcess();    //EEP-ROM�������݊֐�
      /*
      ���̊֐�����Ɏ��s�����悤�ɂ���B
      setPageWriteI2CEeprom�֐��ɂ���ď������ꂽ���̂��������ށB
      */
    }
    else if( lcd_modeview == 1 )
    {
      save_f = 0;

      // ���[�h�\��
      lcd_write(1, "mod:");
      lcd_put_dig(mod, 2);

      // dis_cnt�\��
      lcd_print(" dis:");
      lcd_put_dig(dis_cnt, 5);

      if( ang2 < 0 ){
        ang3 = (-ang2);
      }
      else
      {
        ang3 = ang2;
      }

      lcd_write(2, "ang:");

      if( ang2 < 0){
        lcd_print("-");
      }
      else
      {
        lcd_print(" ");
      }

      lcd_put_dig(ang3, 2);
    }


    //=====�ʏ푖�s=====//
    /* ���[�h�l
        1  ����
       10  ���s
       20  �Z���T�듮�씻��ƃN���X/�n�[�t���C������
        30  �N�����N���[�h
       40  �����[���`�F���W���[�h
       50  �E���[���`�F���W���[�h
    default  mod�l�s������~
    */
    switch(mod){
      //=====1:����=====//
      case 1:
        motor_mode_f( FREE, FREE );
        motor_mode_r( FREE, FREE );

        speed_f(98, 98);
        speed_r(98, 98);

        if( dis_cnt > 620 ){
          dis_cnt = 0;
          cnt7 = 0;
          cnt9 = 0;
          mod = 10;
        }
        break;

      case 10:
        //�g���[�XON
        tr_f = 1;
        dg_tr_f = 1;

        //�⏈������
        error( on, on, on, on );

        //���[�^�[���[�h
        motor_mode_f( FREE, FREE );
        motor_mode_r( FREE, FREE );

        if( !DS_S ){
          if( AS_SAKA <= eep_sak_sensor*10 ){
            BZ = 1;
            sp_enc_f( eep_enc_topbrake, eep_enc_topbrake, on );
            sp_enc_r( eep_enc_topbrake, eep_enc_topbrake, on );
          }
          else{
            if( enc > 65 ){
              cnt7 = 0;
              mod = 11;
            }
            else{
              if( ang2 > eep_curve_angle || ang2 < -eep_curve_angle ){
                BZ = 0;
                if( ang2 > 50 ){
                  speed_f( 0, 0 );
                  speed_r( 0, 0 );
                }else if( ang2 > 10 ){
                  speed_f( diff(eep_curve_speed, F), eep_curve_speed );
                  if( eep_r_diff == 1 ){
                    speed_r( diff(eep_curve_speed, B), eep_curve_speed );
                  }
                }else if( ang2 < -50 ){
                        speed_f( 0, 0 );
                  speed_r( 0, 0 );
                }else if( ang2 < -10 ){
                  speed_f( eep_curve_speed, diff(eep_curve_speed, F) );
                  if( eep_r_diff == 1 ){
                    speed_r( eep_curve_speed, diff(eep_curve_speed, B) );
                  }
                }else {
                  speed_f( eep_curve_speed, eep_curve_speed );
                  if( eep_r_diff == 1 ){
                    speed_r( eep_curve_speed, eep_curve_speed );
                  }
                }
                if( eep_r_diff == 0 ){
                  speed_r( eep_curve_speed, eep_curve_speed );
                }
              }
              else{
                BZ = 0;
                speed_f(eep_enc_speed, eep_enc_speed);
                speed_r(eep_enc_speed, eep_enc_speed);
              }
            }
          }
        }
        else if( DS_S ){
          BZ = 0;
          speed_f(eep_enc_speed, eep_enc_speed);
          speed_r(eep_enc_speed, eep_enc_speed);
        }

        //�n�[�t�E�N���X���C�����o �듮�씻�胂�[�h��
        if( DS_C && ( ( DS_LO && DS_LI ) || ( DS_RI && DS_RO ) ) ){
          sensor = SENSOR;
          ck = 0;
          cnt7 = 0;
          tr_f = 0;
          dis_cnt = 0;
          mod = 20;
        }
        break;

      //=====11:�u���[�L=====//
      case 11:
        BZ = 1;
        tr_f = 1;

        if( ang2 > 40 ){
          speed_f(0, 0);
          speed_r(-20, 0);
        }
        else if( ang2 < -40 ){
          speed_f(0, 0);
          speed_r(0, -20);
        }
        else{
          speed_f(0, 0);
          speed_r(0, 0);
        }

        if( cnt7 > 100 ){
          normal_enc_cnt = 0;
          cnt7 = 0;
          mod = 10;
        }
        break;

      //=====20:���[�h���� �Z���T�[�`�F�b�N=====//
      case 20:
        if( dis_cnt > 960 ){
          cnt7 = 0;
          dis_cnt = 0;
          mod = 10;
          break;
        }

        if( ck < 3 ){
          ck++;
          if( SENSOR != sensor ){
            cnt7 = 0;
            dis_cnt = 0;
            mod = 10;
            break;
          }
        }
        else/* if( dis_cnt >= 10 )*/{
          if( DS_C && DS_LO && DS_LI && DS_RI && DS_RO){
            cnt7 = 0;
            dis_cnt = 0;
            tr_f = 1;
            mod = 30;
            break;  
          }
          //�����[���`�F���W
          else if( DS_LO && DS_LI && DS_C && !DS_RO ){
            cnt7 = 0;
            dis_cnt = 0;
            tr_f = 1;
            mod = 40;
            break;
          }
          //�E���[���`�F���W
          else if( !DS_LO && DS_C && DS_RI && DS_RO ){
            cnt7 = 0;
            dis_cnt = 0;
            tr_f = 1;
            mod = 50;
            break;
          }
        }
        break;

      //=====30:�N�����N���[�h=====//
      case 30://�N���X���C���ʉߏ��� ��{�ړǂݔ�΂�
        BZ = 1;
        crank = 0;

        motor_mode_f( BRAKE, BRAKE );
        motor_mode_r( BRAKE, BRAKE );
        motor_mode_s( BRAKE );

        if( enc > enc_speed_pulse[eep_crank_inspeed + 8] ){
          speed_f(-40, -40);
          speed_r(-40, -40);
        }
        else if( enc > enc_speed_pulse[eep_crank_inspeed + 5] ){
          speed_f(-20, -20);
          speed_r(-20, -20);
        }
        else if( enc > enc_speed_pulse[eep_crank_inspeed] ){
          speed_f(10, 10);
          speed_r(10, 10);
        }
        else{
          speed_f(0, 0);
          speed_r(0, 0);
        }

        if( cnt7 > 180 ){
          BZ = 0;
          cnt7 = 0;
          mod = 31;
        }
        break;

      case 31://�N���X���C����̃g���[�X ���p���o����
        sp_enc_f(eep_crank_inspeed, eep_crank_inspeed, off);
        sp_enc_r(eep_crank_inspeed, eep_crank_inspeed, off);

        if( DS_C && DS_LO && DS_LI && !DS_RO ){      // ���N�����N
          tr_f = off;
          cnt7 = 0;
          dis_cnt = 0;
          crank = L;
          mod = 39;
        }
        else if( DS_C && !DS_LO && DS_RI && DS_RO ){  // �E�N�����N
          tr_f = off;
          cnt7 = 0;
          dis_cnt = 0;
          crank = R;
          mod = 39;
        }
        break;

      case 39://�S�����̈�
        tr_f = 0;
        error( off, on, on, on);

        if( crank == L ){
          motor_mode_f( BRAKE, FREE );
          motor_mode_r( BRAKE, FREE );

          servo_pwm( eep_crank_angle, L);

          speed_f(-90, 0);
          speed_r(-90, 0);

          if( cnt7 > 15 ){
            cnt7 = 0;
            mod = 32;
          }
        }
        else if( crank == R ){
          motor_mode_f( FREE, BRAKE );
          motor_mode_r( FREE, BRAKE );

          servo_pwm( eep_crank_angle, R);

          speed_f(0, -90);
          speed_r(0, -90);

          if( cnt7 > 15 ){
            cnt7 = 0;
            mod = 34;
          }
        }
        break;

      case 32://���N�����N����
        servo_pwm( eep_crank_angle, L);
        if( cnt7 < 20 ){
          speed_f(0, 90);
          speed_r(21, 80);
        }
        else{
          speed_f(25, 90);
          speed_r(21, 70);
        }

        if( cnt7 > 120 ){
          if(( (!DS_LO && DS_LI && !DS_C && !DS_RI && !DS_RO) || (DS_LO && !DS_LI && !DS_C && !DS_RI && !DS_RO) || (DS_LO && DS_LI && !DS_C && !DS_RI && !DS_RO) ) && dis_cnt >300 ){
            mod = 33;
          }
        }
        break;

      case 33://���N�����N����
        servo_pwm( eep_crank_angle, L);
        speed_f(25, 70);
//        speed_r(33, 50);
        speed_r(21, 70);

        if( DS_C ){
          dis_cnt = 0;
          cnt7 = 0;
          mod = 36;
        }
        break;

      case 34://�E�N�����N����
        servo_pwm( eep_crank_angle, R);
        if( cnt7 < 20 ){
          speed_f(90, 0);
          speed_r(80, 21);
        }
        else{
          speed_f(90, 25);
          speed_r(70, 21);
        }

        if( cnt7 > 120 ){
          if(( (!DS_LO && !DS_LI && !DS_C && DS_RI && !DS_RO) || (!DS_LO && !DS_LI && !DS_C && !DS_RI && DS_RO) || (!DS_LO && !DS_LI && !DS_C && DS_RI && DS_RO) )&& dis_cnt > 300 ){
            mod = 35;
          }
        }
        break;

      case 35://�E�N�����N����
         servo_pwm( eep_crank_angle, R);
        speed_f(70, 25);
//        speed_r(50, 33);
        speed_r(70, 21);

        if( DS_C ){
          dis_cnt = 0;
          cnt7 = 0;
          mod = 36;
        }
        break;

      case 36://�ʏ푖�s ���A�O����
        error( off, off, on, on);
        dg_tr_f = 1;
        tr_f = on;
        motor_mode_f( FREE, FREE );
        motor_mode_r( FREE, FREE );

        if( cnt7 <= 50 ){
          pd_angle( pot );
          if( crank == L ){
            //speed_f(90, 70);
            speed_r(diff(80, B), 80);
          }
          else if( crank == R ){
            //speed_f(70, 90);
            speed_r(80, diff(80, B));
          }
        }
        else{
          tr_f = on;
          speed_f(90, 90);
          speed_r(90, 90);
        }

        if( DS_C && cnt7 >= 180 ){
          motor_mode_s( FREE );
          error( on, on, on, on);
          normal_enc_cnt = 0;
          dis_cnt = 0;
          cnt7 = 0;
          mod = 10;
        }
        break;


      //=====40:�����[���`�F���W���[�h=====//
      case 40://�n�[�t���C���ʉߏ��� ��{�ړǂݔ�΂�
        BZ = 1;

        if( DS_C && DS_LO && DS_LI && DS_RI && DS_RO){
          cnt7 = 0;
          dis_cnt = 0;
          tr_f = 1;
          mod = 30;
          break;
        }

        if( cnt7 >= 100 ){
          BZ = 0;
//          dis_cnt = 0;
          cnt7 = 0;
          mod = 41;
        }
        break;

      case 41://�n�[�t���C���ʉߌ�̃g���[�X ���C���������o����
        if( dis_cnt > 1920 ){
          cnt7 = 0;
          dis_cnt = 0;
          mod = 10;
          break;
        }        

        speed_f( eep_lane_inspeed, eep_lane_inspeed );
        speed_r( eep_lane_inspeed, eep_lane_inspeed );

        if( !DS_S && DS_C && !DS_RO && !DS_RI && !DS_LI && !DS_LO && dis_cnt > 500) {
          dis_cnt = 0;
          cnt7 = 0;
          error( off, off, off, off );
          mod = 42;
        }
        break;

      case 42://���[���`�F���W����
        tr_f = off;
        pd_angle( pot - eep_l_lane1 );
        speed_f(85, 75);
        speed_r(85, 75);

        if( (DS_LO && !DS_LI && !DS_C && !DS_RI && !DS_RO || DS_LO && DS_LI && !DS_C && !DS_RI && !DS_RO) && dis_cnt > 200 && cnt7 > 100){
          cnt7 = 0;
          dis_cnt = 0;
          mod = 43;
        }
        break;

      case 43://���[���`�F���W���� �؂�Ԃ�
        tr_f = off;
        pd_angle( pot + eep_l_lane2 );
        speed_f(75, 80);
        speed_r(75, 80);

        if( DS_C && /*dis_cnt > 200 &&*/ cnt7 > 120){
          cnt7 = 0;
          dis_cnt = 0;
          mod = 44;
        }
        break;

      case 44://�ʏ푖�s ���A�O����
        error( on, on, on, on );
        if( cnt7 <= 30 ){
          tr_f = off;
          pd_angle( pot );
        }
        else{
          tr_f = on;
        }

        speed_f(90, 90);
        speed_r(90, 90);

        if( DS_C && cnt7 >= 80 ){
          normal_enc_cnt = 0;
          dis_cnt = 0;
          cnt7 = 0;
          mod = 10;
        }
        break;


      //=====50:�E���[���`�F���W���[�h=====//
      case 50://�n�[�t���C���ʉߏ��� ��{�ړǂݔ�΂�
        BZ = 1;

        if( DS_C && DS_LO && DS_LI && DS_RI && DS_RO){
          cnt7 = 0;
          dis_cnt = 0;
          tr_f = 1;
          mod = 30;
          break;
        }

        if( cnt7 > 100 ){
          BZ = 0;
          cnt7 = 0;
//          dis_cnt = 0;
          mod = 51;
        }
        break;

      case 51://�n�[�t���C���ʉߌ�̃g���[�X ���C���������o����
        if( dis_cnt > 1920 ){
          cnt7 = 0;
          dis_cnt = 0;
          mod = 10;
          break;
        }

        speed_f( eep_lane_inspeed, eep_lane_inspeed );
        speed_r( eep_lane_inspeed, eep_lane_inspeed );

        if( !DS_S && DS_C && !DS_RO && !DS_RI && !DS_LI && !DS_LO && dis_cnt > 500 ){
          dis_cnt = 0;
          cnt7 = 0;
          error( off, off, off, off );
          mod = 52;
        }
        break;

      case 52://���[���`�F���W����
        tr_f = off;
        pd_angle( pot + eep_r_lane1 );
        speed_f(80, 75);
        speed_r(80, 75);
//        speed_f(75, 80);
//        speed_r(75, 80);

        if( (DS_RO && !DS_RI && !DS_C && !DS_LI && !DS_LO || DS_RO && DS_RI && !DS_C && !DS_LI && !DS_LO) && dis_cnt > 200 && cnt7 > 100 ){
          cnt7 = 0;
          dis_cnt = 0;
          mod = 53;
        }
        break;

      case 53://���[���`�F���W���� �؂�Ԃ�
        tr_f = off;
        pd_angle( pot - eep_r_lane2 );
        speed_f(80, 75);
        speed_r(80, 75);

        if( DS_C && /*dis_cnt > 200 && */ cnt7 > 120){
          cnt7 = 0;
          dis_cnt = 0;
          mod = 54;
        }
        break;

      case 54://�ʏ푖�s ���A�O����
        error( on, on, on, on );
        if( cnt7 <= 30 ){
          tr_f = off;
          pd_angle( pot );
        }
        else{
          tr_f = on;
        }

        speed_f(90, 90);
        speed_r(90, 90);

        if( DS_C && cnt7 >= 80 ){
          normal_enc_cnt = 0;
          dis_cnt = 0;
          cnt7 = 0;
          mod = 10;
        }
        break;


      //=====100:��~���[�h=====//
      case 100:
        stop();
        break;


      //=====default:mod�l�s������~=====//
      default:
        stop();
    }
  }
}

//==========================================================================//
//  �O�փ��[�^�[                                                            //
//==========================================================================//
void speed_f(int accele_l, int accele_r)
{
  unsigned long speed_max;

  speed_max = (unsigned long)PWM_CYCLE - 1;

  if( !mov ){
    return;
  }

  //=====�����[�^=====//
  if( accele_l > 0 ){
    PA.DR.BIT.B1 = 0;
  }
  else if( accele_l < 0 ){
    PA.DR.BIT.B1 = 1;
    accele_l = -accele_l;
  }
  if( ITU0.GRB > 20 ){
    while( (ITU0.TCNT >= ITU0.GRB - 20) && (ITU0.TCNT <= ITU0.GRB) );
  }
  else{
    while( (ITU0.TCNT >= ITU0.GRA - 20) && (ITU0.TCNT <= ITU0.GRB) );
  }
  ITU0.GRB = speed_max * accele_l / 100;

  //=====�E���[�^=====//
  if( accele_r > 0 ){
    PA.DR.BIT.B3 = 1;
  }
  else if( accele_r < 0 ){
    PA.DR.BIT.B3 = 0;
    accele_r = -accele_r;
  }
  if( ITU1.GRB > 20 ){
    while( (ITU1.TCNT >= ITU1.GRB - 20) && (ITU1.TCNT <= ITU1.GRB) );
  }
  else{
    while( (ITU1.TCNT >= ITU1.GRA - 20) && (ITU1.TCNT <= ITU1.GRB) );
  }
  ITU1.GRB = speed_max * accele_r / 100;
}

//==========================================================================//
//  ��փ��[�^�[                                                            //
//==========================================================================//
void speed_r(int accele_l, int accele_r)
{
  unsigned long speed_max;

  speed_max = (unsigned long)PWM_CYCLE - 1;

  if( !mov ){
    return;
  }

    //=====�����[�^=====//
    if( accele_l >= 0 ) {
    PB.DR.BIT.B2 = 0;
//        PBDR &= 0xfb;
    } else {
    PB.DR.BIT.B2 = 1;
//       PBDR |= 0x04;
        accele_l = -accele_l;
    }
    ITU3.BRB = speed_max * accele_l / 100;

    //=====�E���[�^=====//
    if( accele_r >= 0 ) {
    PB.DR.BIT.B3 = 1;
//        PBDR &= 0xf7;
    } else {
    PB.DR.BIT.B3 = 0;
//        PBDR |= 0x08;
        accele_r = -accele_r;
  }
    ITU4.BRA = speed_max * accele_r / 100;
}

//==========================================================================//
//  ���[�^���[�G���R�[�_�ɂ��X�s�[�h����                                  //
//==========================================================================//
//=====�O�փ��[�^�[=====//
void sp_enc_f( unsigned char ir, unsigned char il, int d ) 
{
  int pr,pl,tmp;

  if( !mov ){
    return;
  }

  pr =(int)(((enc_speed_pulse[ ir ]*3)/10) + 1*( enc_speed_pulse[ ir ]  - enc)+5);
  pl =(int)(((enc_speed_pulse[ il ]*3)/10) + 1*( enc_speed_pulse[ il ]  - enc)+5);
    //          ��{�X�s�[�h          ��ᐧ��
  if( pr > 99 )   pr = 99;
  if( pr < -95 ) pr = -95;
  if( pl > 99 )   pl = 99;
  if( pl < -95 ) pl = -95;

  /*
  if( pr > 0 ) tim_s = 0;
  if( pr < 0 && tim_s > 70 ) pr=0;
  if( pl > 0 ) tim_s1 = 0;
  if( pl < 0 && tim_s1 > 70 ) pl=0;
  */

  if( d == on ){
    if( ang2 > 50 ){
      speed_f( 0, 0 );
    }else if( ang2 > 10 ){
      speed_f( diff(pl, F), pr );
    }else if( ang2 < -50 ){
            speed_f( 0, 0 );
    }else if( ang2 < -10 ){
      speed_f( pl, diff(pr, F) );
    }else {
      speed_f( pl, pr );
    }
  }
  else{
    speed_f( pl, pr );
  }
  return; 
}

//=====��փ��[�^�[=====//
void sp_enc_r( unsigned char ir, unsigned char il, int d ) 
{
  int pr,pl,tmp;

  if( !mov ){
    return;
  }

  pr =(int)(((enc_speed_pulse[ ir ]*3)/10) + 1*(enc_speed_pulse[ ir ]  - enc)+5);
  pl =(int)(((enc_speed_pulse[ il ]*3)/10) + 1*( enc_speed_pulse[ il ]  - enc)+5);
    //          ��{�X�s�[�h          ��ᐧ��
  if( pr > 99 )   pr = 99;
  if( pr < -95 ) pr = -95;
  if( pl > 99 )   pl = 99;
  if( pl < -95 ) pl = -95;

  /*
  if( pr > 0 ) tim_s = 0;   
  if( pr < 0 && tim_s > 70 ) pr=0;
  if( pl > 0 ) tim_s = 0;   
  if( pl < 0 && tim_s > 70 ) pl=0;
  */

  if( d == on ){
    if( ang2 > 50 ){
      speed_r( 0, 0 );
    }else if( ang2 > 10 ){
      speed_r( diff(pl, B), pr );
    }else if( ang2 < -50 ){
            speed_r( 0, 0 );
    }else if( ang2 < -10 ){
      speed_r( pl, diff(pr, B) );
    }else {
      speed_r( pl, pr );
    }
  }
  else{
    speed_r( pl, pr );
  }
  return; 
}

//==========================================================================//
//�@�����g���[�X PD����                                                     //
//==========================================================================//
void PD_Trace(void)
{
  signed int v_ret;

  //v = ( AS_L - AS_R );
  v = get_AS();

  //v_ret = ( 30 * v ) - ( 220 * (v_before - v ));              //P�|D����ʉ��Z
  v_ret = ( H_TR_KP * v ) - ( H_TR_KD * ( v_before - v ));      //P�|D����ʉ��Z


  if( v > 0 ){                    //�n���h�����O����
    SV_TURN = 0;
  }
  else{
    SV_TURN = 1;
    v_ret = - v_ret;
  }

  //v = v << 3;

  if( v_ret > PWM_CYCLE - 1 ){
    v_ret = PWM_CYCLE - 1;      //����l�ݒ�
  }
  if( v_ret < 200 ){
    v_ret = 0;                  //�T�[�{�����
  }

  ITU4.BRB = v_ret;

  v_before = v;
}

//==========================================================================//
//  PD Angle module                                                         //
//==========================================================================//
void pd_angle(unsigned char pot_tag)
{
  signed int h_ret, h;

  //���C���g���[�XOFF
  tr_f = 0;

  //�Z���T�[�΍����Z
  h = ( PO_S - pot_tag );

  //PD���䉉�Z
  //H_SV_KP �̔��萔 �� 100�`500�@H_SV_KD �̌W�� �� P�̔��萔��5�{�`10�{
  //    P���䉉�Z��      D���䉉�Z��
  h_ret = ( H_SV_KP * h ) - ( H_SV_KD * ( h_before - h ));

  //�n���h�����O�����u���b�N
  if( h > 0 ){
    SV_TURN = 1;
  }
  else{
    SV_TURN = 0;
    h_ret = - h_ret;
  }

  //500�ȉ��̃Q�C����0�Z�b�g
  if( h_ret < 500 ){
    h_ret = 0;
  }

  //�T�[�{����ʂ̏���l�̐���
  if( h_ret >= PWM_CYCLE - 1){
    h_ret = PWM_CYCLE - 1;
  }

  //�T�[�{����ʂ�PWM�o��
  ITU4.BRB = h_ret;

  //�ߋ��̕΍����L��
  h_before = h;

  return;
}

//==========================================================================//
//  ����PWM�v�Z                                                             //
//                                                                          //
//  �O��PWM�������PWM������o���@�X�e�A�����O�p�x�͌��݂̒l���g�p          //
//==========================================================================//
int diff(int pwm, int fr)
{
  int i, ret;

  i = ang2 / 1.2;    //1�x������̑����Ŋ���

  if( i < 0 ){
    i = -i;
  }
  if( i > 45 ){
    i = 45;
  }

  if( fr == F ){
    ret = f_revolution_difference[i] * pwm / 100;
  }
  else if( fr == B ){
    ret = b_revolution_difference[i] * pwm / 100;
  }

  return ret;
}

//==========================================================================//
//  ��~����                                                                //
//==========================================================================//
void stop(void)
{
  if( mov == 0 || !DIP4 ){
    return;
  }

  lcd_clear(0);
  lcd_write(1,"mode:");
  lcd_put_dig(mod,3);
  lcd_print(" Stop!!");

  if( error_num != 0 ){
    lcd_write(2, "Error No.");
    lcd_put_dig(error_num, 1);
  }
  else{
    lcd_write(2, "Finish a race!!");
  }

  motor_mode_f( BRAKE, BRAKE );
  motor_mode_r( BRAKE, BRAKE );

  while(1){
    speed_f(0, 0);
    speed_r(0, 0);

    if( error_num == 0 ){
      BZ = 0;
      PD_Trace();
    }
    else{
      BZ = 1;
      wait10(500);
      BZ = 0;
      wait10(500);

      tr_f = 0;
      ITU4.BRB = 0;
    }
  }
}


//==========================================================================//
//  LCD�\��                                                                 //
//==========================================================================//
//=====���j�^�����O�p�\��=====//
void lcd_data(void)
{
  while(!P_SW4){
    if(P_SW1){
      cl1();
      while(cnt1 <= 10);
      while(P_SW1){
        BZ = 1;
        wait10(25);
        cl1();
      }
      while(cnt1 <= 10);
      if(lcdmode < 2){
        lcdmode++;
      }
      else{
        lcdmode = 0;
      }
      BZ = 0;
      lcd_clear(1);
      lcd_clear(2);
    }

    if(cnt1>=50 && lcdmode == 0){
      if(ang2 < 0){
        ang3 = (-ang2);
      }
      else{
        ang3 = ang2;
      }
      lcd_write(1,"ang:");
      if(ang2 < 0){
        lcd_print("-");
      }
      else{
        lcd_print(" ");
      }
      lcd_put_dig(ang3,3);
      lcd_print(" SAK:");
      lcd_put_dig(AS_SAKA,3);
      lcd_write(2,"");
      lcd_print("ASL:");
      lcd_put_dig(AS_L,3);
      lcd_print("  ASR:");
      lcd_put_dig(AS_R,3);
      lcd_home(1);
      cl1();

      if( DS_SAKA ){
        BZ = 1;
      }
      else{
        BZ = 0;
      }

      if(P_SW2){
        ang1=PO_S;
      }
    }
    else if(cnt1>=50 && lcdmode == 1){
      lcd_write(1,"DS:LLSCRR Trace");
      lcd_write(2,"   ");
      lcd_put_dig(DS_LO,1);
      lcd_put_dig(DS_LI,1);
      lcd_put_dig(DS_S,1);
      lcd_put_dig(DS_C,1);
      lcd_put_dig(DS_RI,1);
      lcd_put_dig(DS_RO,1);
      if( tr_f ){
        lcd_print("    ON");
      }
      else{
        lcd_print("   OFF");
      }

      if( P_SW2 && cnt2 > 150 ){
        if( tr_f ){
          tr_f = 0;
          BZ = 1; wait10(100); BZ = 0; wait10(25);
          BZ = 1; wait10(100); BZ = 0; wait10(25);
        }
        else{
          tr_f = 1;
          BZ = 1;
          wait10(150);
        }
        cl2();
      }
      BZ = 0;
      lcd_home(1);
      cl1();
    }
    else if(cnt1>=50 && lcdmode == 2){
      lcd_write(1,"ENC:");
      lcd_put_dig(enc,3);
      lcd_write(2,"cnt:");
      lcd_put_dig(en_cn,6);
      lcd_home(1);

      if(P_SW2){
        en_cn=0;
      }
    }
  }//while(!P_SW4)
}//lcd_data(void)

//==========================================================================//
//  ���s�p�����[�^�ݒ�                                                      //
//==========================================================================//
//=====�p�����[�^�ݒ�p�\��=====//
void param_set(void)
{
  int param_write;

  while(1){
    tr_f = 0;
    ITU4.BRB = 0;

    if(P_SW1){
      cl1();
      while(cnt1 <= 10);
      while(P_SW1){
        cl1();
      }
      while(cnt1 <= 10);
      if(lcdmode > 0 && lcdmode < 9){
        lcdmode++;
      }
      else{
        lcdmode = 0;
      }

      select_item = 0;

      lcd_clear(1);
      lcd_clear(2);
    }

    //=====Page0:EEP-ROM�L�^�p�����[�^�̓ǂݍ���=====//
    if(cnt1>=50 && lcdmode == 0){
      lcd_write(1, "Load Parameters?");
      lcd_write(2, " Yes  No");

      //=====���ڑI���J�[�\���̕\��=====//
      if( !select_item ){
        lcd_cursor(2, 1);
        lcd_print(">");
      }
      else{
        lcd_cursor(2, 6);
        lcd_print(">");
      }

      //=====�I�����ڂ̕ύX=====//
      if( P_SW3 && cnt2 > 250 ){
        if( select_item ){
          select_item = 0;
        }
        cl2();
      }
      if( P_SW4 && cnt2 > 250 ){
        if( !select_item ){
          select_item = 1;
        }
        cl2();
      }

      //=====�ݒ�ǂݍ��ݏ�������====//
      if( P_SW2 && cnt2 > 250 ){
        lcd_clear(0);

        //=====�ǂݍ��݂���====//
        if( !select_item ){
          param_load();

          lcdmode = 1;
        }
        //=====�ǂݍ��݂Ȃ�====//
        else{
          lcd_write(1, "Not Loaded.");

          lcdmode = 1;
        }

        wait10(150);

        select_item = 0;
        cl2();
      }

      lcd_home(1);
      cl1();
    }

    //=====Page1:�ʏ푖�s=====//  
    if(cnt1>=50 && lcdmode == 1){
      lcd_write(1, "1)     ");

      lcd_print("Speed:");
      lcd_put_dig(eep_enc_speed, 3);
      lcd_write(2, "Normal  ");

      lcd_cursor(1, 7);
      lcd_print(">");

      //=====�I�����ڂ̒l�ύX=====//
      if( P_SW3 && cnt2 > 100 ){
        if( eep_enc_speed > 0 ){
          eep_enc_speed--;
        }
        cl2();
      }
      if( P_SW4 && cnt2 > 100 ){
        if( eep_enc_speed < 99 ){
          eep_enc_speed++;
        }
        cl2();
      }

      lcd_home(1);
      cl1();
    }

    //=====Page2:�J�[�u���s��=====//  
    else if(cnt1>=50 && lcdmode == 2){
      lcd_write(1, "2)      ");

      lcd_print("Angle:");
      lcd_put_dig(eep_curve_angle, 2);

      lcd_write(2, "USteer ");
      lcd_print("Speed:");
      lcd_put_dig(eep_curve_speed, 3);

      //=====���ڑI���J�[�\���̕\��=====//
      if( !select_item ){
        lcd_cursor(1, 8);
        lcd_print(">");
      }
      else{
        lcd_cursor(2, 7);
        lcd_print(">");
      }

      //=====�I�����ڂ̕ύX=====//
      if( P_SW2 && cnt2 > 150 ){
        if( !select_item ){
          select_item = 1;
        }
        else{
          select_item = 0;
        }
        cl2();
      }

      //=====�I�����ڂ̒l�ύX=====//
      if( P_SW3 && cnt2 > 100 ){
        if( !select_item ){
          if( eep_curve_angle > 0 ){
            eep_curve_angle--;
          }
        }
        else{
          if( eep_curve_speed > 0 ){
            eep_curve_speed--;
          }
        }
        cl2();
      }
      if( P_SW4 && cnt2 > 100 ){
        if( !select_item ){
          if( eep_curve_angle < 60 ){
            eep_curve_angle++;
          }
        }
        else{
          if( eep_curve_speed < 99 ){
            eep_curve_speed++;
          }
        }
        cl2();
      }

      lcd_home(1);
      cl1();
    }

    //=====Page3:�N�����N���[�h=====//  
    else if(cnt1>=50 && lcdmode == 3){
      lcd_write(1, "3)      ");

      lcd_print("Speed:");
      lcd_put_dig(eep_crank_inspeed, 2);

      lcd_write(2, "Crank     ");
      lcd_print("Ang:");
      lcd_put_dig(eep_crank_angle, 2);

      //=====���ڑI���J�[�\���̕\��=====//
      if( !select_item ){
        lcd_cursor(1, 8);
        lcd_print(">");
      }
      else{
        lcd_cursor(2, 8);
        lcd_print(">");
      }

      //=====�I�����ڂ̕ύX=====//
      if( P_SW2 && cnt2 > 150 ){
        if( !select_item ){
          select_item = 1;
        }
        else{
          select_item = 0;
        }
        cl2();
      }

      //=====�I�����ڂ̒l�ύX=====//
      if( P_SW3 && cnt2 > 100 ){
        if( !select_item ){
          if( eep_crank_inspeed > 0 ){
            eep_crank_inspeed--;
          }
        }
        else{
          if( eep_crank_angle > 0 ){
            eep_crank_angle --;
          }
        }
        cl2();
      }
      if( P_SW4 && cnt2 > 100 ){
        if( !select_item ){
          if( eep_crank_inspeed < 50 ){
            eep_crank_inspeed++;
          }
        }
        else{
          if( eep_crank_angle < 100 ){
            eep_crank_angle ++;
          }
        }
        cl2();
      }

      lcd_home(1);
      cl1();
    }

    //=====Page4:���[���`�F���W�X�s�[�h=====//
    else if(cnt1>=50 && lcdmode == 4){
      lcd_write(1, "4) LaneChange");
      lcd_write(2, "       >Speed:");
      lcd_put_dig(eep_lane_inspeed, 2);

      //=====�I�����ڂ̒l�ύX=====//
      if( P_SW3 && cnt2 > 100 ){
        if( eep_lane_inspeed > 0 ){
          eep_lane_inspeed--;
        }
        cl2();
      }
      if( P_SW4 && cnt2 > 100 ){
        if( eep_lane_inspeed < 99 ){
          eep_lane_inspeed++;
        }
        cl2();
      }

      lcd_home(1);
      cl1();
    }

    //=====Page5:���[���`�F���W �� �؂�p�ݒ�=====//
    else if(cnt1>=50 && lcdmode == 5){
      lcd_write(1, "5)       ");

      lcd_print("Ang1:");
      lcd_put_dig(eep_l_lane1, 2);

      lcd_write(2, "LaneAngL ");
      lcd_print("Ang2:");
      lcd_put_dig(eep_l_lane2, 2);

      //=====���ڑI���J�[�\���̕\��=====//
      if( !select_item ){
        lcd_cursor(1, 9);
        lcd_print(">");
      }
      else{
        lcd_cursor(2, 9);
        lcd_print(">");
      }

      //=====�I�����ڂ̕ύX=====//
      if( P_SW2 && cnt2 > 150 ){
        if( !select_item ){
          select_item = 1;
        }
        else{
          select_item = 0;
        }
        cl2();
      }

      //=====�I�����ڂ̒l�ύX=====//
      if( P_SW3 && cnt2 > 100 ){
        if( !select_item ){
          if( eep_l_lane1 > 0 ){
            eep_l_lane1--;
          }
        }
        else{
          if( eep_l_lane2 > 0 ){
            eep_l_lane2--;
          }
        }
        cl2();
      }
      if( P_SW4 && cnt2 > 100 ){
        if( !select_item ){
          if( eep_l_lane1 < 60 ){
            eep_l_lane1++;
          }
        }
        else{
          if( eep_l_lane2 < 60 ){
            eep_l_lane2++;
          }
        }
        cl2();
      }

      lcd_home(1);
      cl1();
    }

    //=====Page6:���[���`�F���W �E �؂�p�ݒ�=====//
    else if(cnt1>=50 && lcdmode == 6){
      lcd_write(1, "6)       ");

      lcd_print("Ang1:");
      lcd_put_dig(eep_r_lane1, 2);

      lcd_write(2, "LaneAngR ");
      lcd_print("Ang2:");
      lcd_put_dig(eep_r_lane2, 2);

      //=====���ڑI���J�[�\���̕\��=====//
      if( !select_item ){
        lcd_cursor(1, 9);
        lcd_print(">");
      }
      else{
        lcd_cursor(2, 9);
        lcd_print(">");
      }

      //=====�I�����ڂ̕ύX=====//
      if( P_SW2 && cnt2 > 150 ){
        if( !select_item ){
          select_item = 1;
        }
        else{
          select_item = 0;
        }
        cl2();
      }

      //=====�I�����ڂ̒l�ύX=====//
      if( P_SW3 && cnt2 > 100 ){
        if( !select_item ){
          if( eep_r_lane1 > 0 ){
            eep_r_lane1--;
          }
        }
        else{
          if( eep_r_lane2 > 0 ){
            eep_r_lane2--;
          }
        }
        cl2();
      }
      if( P_SW4 && cnt2 > 100 ){
        if( !select_item ){
          if( eep_r_lane1 < 60 ){
            eep_r_lane1++;
          }
        }
        else{
          if( eep_r_lane2 < 60 ){
            eep_r_lane2++;
          }
        }
        cl2();
      }

      lcd_home(1);
      cl1();
    }

    //=====Page7:�⓹���s�ݒ�=====//
    else if(cnt1>=50 && lcdmode == 7){
      lcd_write(1, "7) Slope  ");

      lcd_print("Brk:");
      lcd_put_dig(eep_enc_topbrake, 2);

      lcd_write(2, "      ");
      lcd_print(" R_Diff:");
      lcd_put_dig(eep_r_diff, 2);

      //=====���ڑI���J�[�\���̕\��=====//
      if( !select_item ){
        lcd_cursor(1, 10);
        lcd_print(">");
      }
      else{
        lcd_cursor(2, 7);
        lcd_print(">");
      }

      //=====�I�����ڂ̕ύX=====//
      if( P_SW2 && cnt2 > 150 ){
        if( !select_item ){
          select_item = 1;
        }
        else{
          select_item = 0;
        }
        cl2();
      }

      //=====�I�����ڂ̒l�ύX=====//
      if( P_SW3 && cnt2 > 80 ){
        if( !select_item ){
          if( eep_enc_topbrake > 0 ){
            eep_enc_topbrake--;
          }
        }
        else{
          if( eep_r_diff > 0 ){
            eep_r_diff--;
          }
        }
        cl2();
      }
      if( P_SW4 && cnt2 > 80 ){
        if( !select_item ){
          if( eep_enc_topbrake < 70 ){
            eep_enc_topbrake++;
          }
        }
        else{
          if( eep_r_diff < 1 ){
            eep_r_diff++;
          }
        }
        cl2();
      }

      lcd_home(1);
      cl1();
    }

    //=====Page8:�⓹���o�A�i���O�Z���T�[�l������s�����ݒ�=====//
    else if(cnt1>=50 && lcdmode == 8){
      lcd_write(1, "8)   ");

      lcd_print("SAK_CdS:");
      lcd_put_dig(eep_sak_sensor*10, 3);

      lcd_write(2, "      ");
      lcd_print("Dis:");
      lcd_put_dig(eep_run_dis, 3);
      lcd_print("(m)");

      //=====���ڑI���J�[�\���̕\��=====//
      if( !select_item ){
        lcd_cursor(1, 5);
        lcd_print(">");
      }
      else{
        lcd_cursor(2, 6);
        lcd_print(">");
      }

      //=====�I�����ڂ̕ύX=====//
      if( P_SW2 && cnt2 > 150 ){
        if( !select_item ){
          select_item = 1;
        }
        else{
          select_item = 0;
        }
        cl2();
      }

      //=====�I�����ڂ̒l�ύX=====//
      if( P_SW3 && cnt2 > 80 ){
        if( !select_item ){
          if( eep_sak_sensor > 0 ){
            eep_sak_sensor--;
          }
        }
        else{
          if( eep_run_dis > 0 ){
            eep_run_dis -= 5;
          }
        }
        cl2();
      }
      if( P_SW4 && cnt2 > 80 ){
        if( !select_item ){
          if( eep_sak_sensor < 25 ){
            eep_sak_sensor++;
          }
        }
        else{
          if( eep_run_dis < 125 ){
            eep_run_dis += 5;
          }
        }
        cl2();
      }

      lcd_home(1);
      cl1();
    }

    //=====Page9:�ݒ�I�� �p�����[�^��EEP-ROM�ւ̕ۑ�=====//  
    else if(cnt1>=50 && lcdmode == 9){
      lcd_write(1, "Save Parameters?");
      lcd_write(2, " Yes  No");

      //=====���ڑI���J�[�\���̕\��=====//
      if( !select_item ){
        lcd_cursor(2, 1);
        lcd_print(">");
      }
      else{
        lcd_cursor(2, 6);
        lcd_print(">");
      }

      //=====�I�����ڂ̕ύX=====//
      if( P_SW3 && cnt2 > 250 ){
        if( select_item ){
          select_item = 0;
        }
        cl2();
      }
      if( P_SW4 && cnt2 > 250 ){
        if( !select_item ){
          select_item = 1;
        }
        cl2();
      }

      //=====�ݒ�ۑ���������====//
      if( P_SW2 && cnt2 > 250 ){
        lcd_clear(0);

        //=====�ۑ�����====//
        if( !select_item ){
          lcd_write(1, "Saving...");

          parameter[0] = eep_enc_speed;
          parameter[1] = eep_curve_angle;
          parameter[2] = eep_curve_speed;
          parameter[3] = eep_crank_inspeed;
          parameter[4] = eep_crank_angle;
          parameter[5] = eep_lane_inspeed;
          parameter[6] = eep_l_lane1;
          parameter[7] = eep_l_lane2;
          parameter[8] = eep_r_lane1;
          parameter[9] = eep_r_lane2;
          parameter[10] = eep_r_diff;
          parameter[11] = eep_enc_topbrake;
          parameter[12] = 0;
          parameter[13] = 0;
          parameter[14] = eep_sak_sensor;
          parameter[15] = eep_run_dis;

          setPageWriteI2CEeprom(0, 16, parameter);

          for( param_write = 0; param_write < 30; param_write++ ){
            I2CEepromProcess();
          }

          lcd_write(2, "Done!");
          BZ = 1; wait10(250); BZ = 0; wait10(80);
          BZ = 1; wait10(250); BZ = 0; wait10(80);
          break;
        }
        //=====�ۑ��Ȃ�====//
        else{
          lcd_write(1, "Not saved.");

          BZ = 1; wait10(150); BZ = 0; wait10(50);
          BZ = 1; wait10(150); BZ = 0; wait10(50);
          break;
        }
      }

      lcd_home(1);
      cl1();
    }
  }//while(1)
}//param_set(void)


//==========================================================================//
//  �p�����[�^�l������                                                      //
//==========================================================================//
void param_init(void)
{
  stop_dis = (long)(eep_run_dis / (0.033 * 3.14159)) * 200;
}

//==========================================================================//
//  �p�����[�^�l�ǂݍ���                                                    //
//==========================================================================//
void param_load(void)
{
  lcd_write(1, "Loading...");

  eep_enc_speed = readI2CEeprom(0);
  eep_curve_angle = readI2CEeprom(1);
  eep_curve_speed = readI2CEeprom(2);
  eep_crank_inspeed = readI2CEeprom(3);
  eep_crank_angle = readI2CEeprom(4);
  eep_lane_inspeed = readI2CEeprom(5);
  eep_l_lane1 = readI2CEeprom(6);
  eep_l_lane2 = readI2CEeprom(7);
  eep_r_lane1 = readI2CEeprom(8);
  eep_r_lane2 = readI2CEeprom(9);
  eep_r_diff = readI2CEeprom(10);
  eep_enc_topbrake = readI2CEeprom(11);
//  eep_tr_kp = readI2CEeprom(12);
//  eep_tr_kd = readI2CEeprom(13);
  eep_sak_sensor = readI2CEeprom(14);
  eep_run_dis = readI2CEeprom(15);

  lcd_write(2, "Done!");
  wait10(500);
}

//==========================================================================//
//  �T�[�{PWM����                                                           //
//==========================================================================//
void servo_pwm(int pwm, int lr)
{
  unsigned long max_speed;

  max_speed = PWM_CYCLE - 1;

  if( lr == L ){
    SV_TURN = L;
  }
  else if( lr == R ){
    SV_TURN = R;
  }

    ITU4.BRB = max_speed * pwm / 100;
}

//==========================================================================//
//�@���荞�ݏ���                                                            //
//==========================================================================//
#pragma interrupt(wait0_1ms)
void wait0_1ms(void)
{
  ITU0.TSR.BYTE &= 0xf8;

  ang2 = ang1 - PO_S;        //���ݒn�擾

  //=====���C���g���[�X=====//  
  if( tr_f ){
    PD_Trace();
  }

  //=====�ėp�J�E���^=====//
  cnt1++;
  cnt2++;
  cnt3++;
  cnt4++;
  cnt5++;
  cnt6++;
  cnt7++;
  cnt8++;
  cnt9++;

//  tim_s++;
//  tim_s1++;

  ck_cnt++;              //�`�F�b�N�J�E���^  
  stop_cnt++;              //���Ԓ�~�J�E���^

  //=====���[�^���[�G���R�[�_ �J�E���g=====//
  counter++;
  if( counter >= 10 ){ //10ms���Ƒ���
    now_enc = ITU2.TCNT;
    enc = now_enc;

    enc_cnt += now_enc;          //10ms���p���X�i�[�J�E���^
    dis_cnt += now_enc;          //�ėp�����v���p�J�E���^
    en_cn += now_enc;          //�ėp�����v���p�J�E���^
    stop_cnt += now_enc;        //��~�����v���p�J�E���^
    if( mod == 1 || mod == 10 ){
      normal_enc_cnt += now_enc;
    }
    ITU2.TCNT = 0;
    counter = 0;
  }


  //=====EEP-ROM �������݃f�[�^�Z�b�g=====//
  cnt_write++;
  if(cnt_write >= 10){
    //=====EEP-ROM�ۑ�����(10ms����)=====//
    if(save_f){
      //�f�W�^���Z���T DS:LO:LI:DC:RC:RO
      Digital = ((P7.DR.BYTE>>2) & 0x3c) + (P3.DR.BYTE & 0x03);

      //���[�^�[����
      f_diff = fl_mo - fr_mo;
      b_diff = bl_mo - br_mo;

      //�L�^�p�z��ւ̒l�i�[
      m_dat[0] = mod;        //���[�h�l
      m_dat[1] = (char)AS_L;      //�A�i���O�Z���T��
      m_dat[2] = (char)AS_R;      //�A�i���O�Z���T�E
      m_dat[3] = (char)PO_S;      //�|�e���V���Z���T
      m_dat[4] = enc;        //���[�^���G���R�[�_
      m_dat[5] = (char)AS_SAKA;//(char)f_diff;  //�O�փ��[�^�[����
      m_dat[6] = DS_SAKA;  //��փ��[�^�[����
      m_dat[7] = Digital;      //�f�W�^���Z���T

      setPageWriteI2CEeprom(addr, 8, m_dat);
      addr += 8;
      if(addr >= 0x8000){
        BZ = 1;
        save_f = 0;
      }
    }//EEP-ROM�ۑ�����(10ms����)
    cnt_write = 0;
  }//EEP-ROM �������݃f�[�^�Z�b�g


  //=====�����ɂ���~=====//
  if( stop_cnt >= stop_dis && mod == 10 && (ang2 > -10 && ang2 < 10 )){
    stop();
  }


  //=====�ُ퓮�쎞��~����=====//
  error_cnt1++;
  error_cnt2++;
  error_cnt3++;
  error_cnt4++;


  //=====�S�Z���T��莞�Ԕ����E�������Œ�~=====//
  if(er1 && (( DS_C && DS_LI && DS_RI && DS_LO && DS_RO) || (!DS_C && !DS_LI && !DS_RI && !DS_LO && !DS_RO))){
    if( error_cnt1 >= 100 ){  //100��200 200��400
      error_num = 1;
      stop();
    }
  }
  else{
    error_cnt1 = 0;
  }

  //=====�G���R�[�_����]�E����]�ɂ���~=====//
  if(er2 && cnt9>=1000 && (enc<=20 || enc>=150)){
    if(error_cnt2 >= 100){    // 150��350��450
      error_num = 2;
      stop();
    }
  }
  else{
    error_cnt2 = 0;
  }

  //=====���E�Z���T��莞�Ԕ�����~=====//
  if(er3 && DS_LO && DS_RO){
    if(error_cnt3 >= 150){    //200��300��400
      error_num = 3;
      stop();
    }
  }
  else{
    error_cnt3 = 0;
  }
}

//==========================================================================//
//  �J�E���g�N���A�֐�                                                      //
//==========================================================================//
void cl1(void)
{
  cnt1 = 0;
}
void cl2(void)
{
  cnt2 = 0;
}
void cl3(void)
{
  cnt3 = 0;
}
void cl4(void)
{
  cnt4 = 0;
}
void cl5(void)
{
  cnt5 = 0;
}
void cl6(void)
{
  cnt6 = 0;
}
void cl7(void)
{
  cnt7 = 0;
}
void cl8(void)
{
  cnt8 = 0;
}
void cl9(void)
{
  cnt9 = 0;
}
void all_cl(void)
{
  cnt1 = 0;
  cnt2 = 0;
  cnt3 = 0;
  cnt4 = 0;
  cnt5 = 0;
  cnt6 = 0;
  cnt7 = 0;
  cnt8 = 0;
  cnt9 = 0;
}

//==========================================================================//
//  ���s�f�[�^ PC�o��                                                       //
//==========================================================================//
void run_data(void)
{
  unsigned long dp, addr;
  dp = 0;

  ITU4.BRB = 0;

  lcd_clear(0);
  lcd_write(1, "Data Output");
  lcd_write(2, "P_SW1 Start");
  while(!P_SW1);            //P_SW1�������ꂽ��J�n

  lcd_clear(0);
  lcd_write(1, "Data Sending...");
  LED10 = 0;

  sci_strtx("\n\n\r");
  sci_strtx("EEP-ROM Data Reading...");

  sci_strtx("\n\n\r");
  sci_strtx("Time, mode, AS_L, AS_R, diff, ang2, enc, F_diff, B_diff, digital\n");
  sci_strtx("\n\n\r");

  //=====EEP-ROM �ǂݏo��=====//
  for(addr=16; readI2CEeprom(addr) != 0; addr+=8){
    if( addr > 32767){
      break;
    }
    sci_put_dig((addr-16)/8, 6);
    sci_strtx(",");
    sci_put_dig((unsigned char)readI2CEeprom(addr), 3);
    sci_strtx(",");
    sci_put_dig((unsigned char)readI2CEeprom(addr+1), 3);
    sci_strtx(",");
    sci_put_dig((unsigned char)readI2CEeprom(addr+2), 3);
    sci_strtx(",");
    sci_put_dig((unsigned char)readI2CEeprom(addr+3), 3);
    sci_strtx(",");
    sci_put_dig((unsigned char)readI2CEeprom(addr+4), 3);
    sci_strtx(",");
    sci_put_dig((unsigned char)readI2CEeprom(addr+5), 3);
    sci_strtx(",");
    sci_put_dig((unsigned char)readI2CEeprom(addr+6), 3);
    sci_strtx(",");
    sci_put_dig((unsigned char)readI2CEeprom(addr+7), 3);
    sci_strtx(",");

    sci_tx('\r');
    sci_tx('\n');
  }//EEP-ROM �ǂݏo��

  //=====EEP-ROM �ǂݏo���̏I��=====//
  sci_strtx("EEP-ROM Data Reading completion");
  sci_strtx("\n\n\r");

  lcd_write(2, "ALL Sending!!");
  LED10 = 1;
  while(1){
    BZ = 1;
    wait10(500);
    BZ = 0;
    wait10(500);
  }
}
/*
���l�\����
1  �ԍ�
2  mod�̒l
3  AS_L
4  AS_R
5  ���E�A�i���O��
6  pot
10  Encoder
7  �O�փ��[�^�[����
8  ��փ��[�^�[����
9  �f�W�^���Z���T
*/

//==========================================================================//
//  SCI ���j�^�����O�i�n�C�p�[�^�[�~�i���j                                  //
//==========================================================================//
void sci_monitor(void)
{
  lcd_clear(0);
  lcd_write(1, "SCI Monitor");
  lcd_write(2, "Look PC Monitor");

  sci_strtx("PO_S  ang2  AS_L  AS_R  DIR  DIGITAL  ENC  ENC_CNT\n");

  while(1){
    wait10(10);
    sci_strtx("\r");

    LED8 = ~LED8;  //�l���]

    //=====�|�e���V���Z���T=====//
    sci_put_num(PO_S, 4);
    sci_strtx("  ");

    //=====���݊p�x=====//
    sci_put_num(ang2, 4);
    sci_strtx("  ");

    //=====�A�i���O�Z���T��=====//
    sci_put_num(AS_L, 4);
    sci_strtx("  ");

    //=====�A�i���O�Z���T�E=====//
    sci_put_num(AS_R, 4);
    sci_strtx("  ");

    //=====�A�i���O�Z���T��=====//
    sen = AS_L - AS_R;
    sci_put_num(sen, 4);
    sci_strtx("  ");

    //=====�f�W�^���Z���T=====//
    sci_put_num(DS_RO, 1);
    sci_put_num(DS_RI, 1);
    sci_put_num(DS_S,  1);
    sci_put_num(DS_C,  1);
    sci_put_num(DS_LI, 1);
    sci_put_num(DS_LO, 1);
    sci_strtx("   ");

    //=====�G���R�[�_�l=====//
    sci_put_num(enc, 3);
    sci_strtx("  ");

    //=====�G���R�[�_�J�E���^=====//
    sci_put_num(en_cn, 8);

    if(P_SW1){
      en_cn = 0;
    }
  }
}

//==========================================================================//
//  �p�����[�^�`�F�b�N PC�o��                                               //
//==========================================================================//
void param_check(void)
{
  unsigned long dp, addr;
  dp = 0;

  lcd_clear(0);
  lcd_write(1, "Parameter Check");
  lcd_write(2, "P_SW1 Start");
  while(!P_SW1);            //P_SW1�������ꂽ��J�n

  lcd_clear(0);
  lcd_write(1, "Data Sending...");
  LED10 = 0;

  sci_strtx("\n\n\r");
  sci_strtx("EEP-ROM Data Reading...");

  //=====EEP-ROM �ǂݏo��=====//
  for(addr=0; addr <= 18; addr++){
    if( addr > 32767){
      break;
    }
    sci_put_dig((unsigned char)readI2CEeprom(addr),5);
    sci_strtx(",");

    sci_tx('\r');
    sci_tx('\n');
  }//EEP-ROM �ǂݏo��

  //=====EEP-ROM �ǂݏo���̏I��=====//
  sci_strtx("EEP-ROM Data Reading completion");
  sci_strtx("\n\n\r");

  lcd_write(2, "ALL Sending!!");
  LED10 = 1;
  while(1){
    BZ = 1;
    wait10(500);
    BZ = 0;
    wait10(500);
  }
}

//==========================================================================//
//  �A�i���O�Z���T�l�擾                                                    //
//==========================================================================//
int get_AS(void)
{
  int ret;

  ret = AS_L - AS_R;

  if( ang2 > 3 || ang2 < -3 ){
    switch( sensor_pattern ){
      case 0:
        //DS_RI
        if( !DS_LO && !DS_LI && !DS_C && DS_RI && !DS_RO ){
          ret = 180;
          break;
        }

        //DS_LI
        if( !DS_LO && DS_LI && !DS_C && !DS_RI && !DS_RO ){
          ret = -180;
          break;
        }

        //DS_RI DS_RO
        if( !DS_LO && !DS_LI && !DS_C && DS_RI && DS_RO ){
          ret = 200;
          sensor_pattern = 1;
          break;
        }

        //DS_LO DS_LI
        if( DS_LO && DS_LI && !DS_C && !DS_RI && !DS_RO ){
          ret = -200;
          sensor_pattern = 2;
          break;
        }
      break;

      case 1://�Z���T�����
        ret = 200;
        if( !DS_LO && !DS_LI && !DS_C && DS_RI && !DS_RO ){
          sensor_pattern = 0;
        }
        break;

      case 2://�Z���T�E���
        ret = -200;
        if( !DS_LO && DS_LI && !DS_C && !DS_RI && !DS_RO ){
          sensor_pattern = 0;
        }
        break;
    }
  }
  else{
    sensor_pattern = 0;
  }

  return ret;
}

//==========================================================================//
//  ��~�������� ON/OFF                                                     //
//==========================================================================//
void error(int er_1, int er_2, int er_3, int er_4)
{
  if( er_1 == on ){
    er1 = 1;
  }
  else if( er_1 == off ){
    er1 = 0;
  }

  if( er_2 == on ){
    er2 = 1;
  }
  else if( er_2 == off ){
    er2 = 0;
  }
  if( er_3 == on ){
    er3 = 1;
  }
  else if( er_3 == off ){
    er3 = 0;
  }

  if( er_4 == on ){
    er4 = 1;
  }
  else if( er_4 == off ){
    er4 = 0;
  }
}

//==========================================================================//
//  ���[�^�[���[�h�ύX �T�[�{���[�^�[                                       //
//==========================================================================//
void motor_mode_s(int mode)
{
  if( mode == FREE ){
    SV_B = 1;
  }
  else{
    SV_B = 0;
  }
}

//==========================================================================//
//  ���[�^�[���[�h�ύX �O�փ��[�^�[                                         //
//==========================================================================//
void motor_mode_f(int mode_l, int mode_r)
{
  if( mode_l == FREE ){
    FL_B = 1;
  }
  else{
    FL_B = 0;
  }

  if( mode_r == FREE ){
    FR_B = 1;
  }
  else{
    FR_B = 0;
  }
}

//==========================================================================//
//  ���[�^�[���[�h�ύX ��փ��[�^�[                                         //
//==========================================================================//
void motor_mode_r(int mode_l, int mode_r)
{
  if( mode_l == FREE ){
    RL_B = 1;
  }
  else{
    RL_B = 0;
  }

  if( mode_r == FREE ){
    RR_B = 1;
  }
  else{
    RR_B = 0;
  }
}
