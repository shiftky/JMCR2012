// �f�[�^�]���p�w�b�_
// �����ݒ�@init_sci1(SMR,BRR);
// ������]���@sci1_strtx("������");
// �����]���@sci_put_dig(���l,����);

void init_sci( char , char );
unsigned char in_sci( void );
void sci_tx( char );
void sci_strtx(char *);
void sci_put_dig( long , char );
void sci_change_num( char );
void sci_put_num( long , char );

// �����ݒ�
void init_sci( char smr, char brr )
{
    int i;
    
	SCI1.SCR.BYTE = 0x00;	// ��~
	SCI1.SMR.BYTE = smr;		// SMR�̒l
	SCI1.BRR = brr;		// BRR�̒l
	for(i=0;i<10000;i++);	// �E�F�C�g
	SCI1.SCR.BYTE = 0x30;	// Tx,Rx����,���荞�݂Ȃ�
	SCI1.SSR.BYTE &= 0x80;	// �G���[�t���O
}

// �f�[�^��M
unsigned char in_sci(void)
{
	unsigned char	data;

	while( !(SCI1.SSR.BYTE & 0x78 ));		// ��M�҂�
	switch( SCI1.SSR.BIT.RDRF ){		// ���V�[�u�t���O�`�F�b�N
		case 1:
			data = SCI1.RDR;		// ��M�f�[�^���
			SCI1.SSR.BIT.RDRF = 0;	// �t���O�N���A
			return( data );
   			break;
		default:				// ���V�[�u�G���[
			SCI1.SSR.BYTE &= 0x87;	// �G���[�t���O�N���A
			return( 0xff );		// �G���[���@0Xff�@�߂�l
	}
}

// �f�[�^�]��(1byte)
void sci_tx(char data)
{
	while(SCI1.SSR.BIT.TDRE == 0);	// �󂫑҂�
	SCI1.TDR = data;				// �f�[�^�Z�b�g
	SCI1.SSR.BIT.TDRE = 0;			// �t���O�N���A
}

// ������]��
void sci_strtx(char *str)
{
	while(*str != '\0'){
		sci_tx(*str);
		str++;
	}
}
void sci_put_num(long num, char keta)
{
    char temp_chr[11];
    int  i,a,x,y,kari;

	if(keta>10)	return; // Error over 10

    i = 0;  a = 0;  x = 1; y = 0; kari = num;

    if(kari == 0) a++;
    else{
    	while( kari != 0 ){
    		kari /= 10;
    		a++;
    		}
    	}
    
    for(i = 1 ;i < a ; i++) x *= 10;
    
    for(i=1 ; i<=keta ; i++){
    	if(i <= keta-a) temp_chr[i-1] = 0x20;
    	else{
    		if( num < 0 ){
    			num=-(num);
    			temp_chr[i-2]='-';
    		}
    		y=num/x;
    		temp_chr[i-1]=(0x30)|(char)y;
    		num=num-(y*x);
    		x/=10;
    	}
    }
    	
    temp_chr[i-1]='\0';
    
    sci_strtx(temp_chr);
    
    //i=0;
    //while(temp_chr[i]!='\0'){
    //	sci_change_num(  temp_chr[i] );
    //	i++;
    //}	
   
}
// ���l�]��,�}�C�i�X���Ή�,�v����
void sci_put_dig(long num, char keta)
{
    char temp_chr[11];
    int  i,a,x,y,kari;

	if(keta>10)	return; // Error over 10

    i = 0;  a = 0;  x = 1; y = 0; kari = num;

    if(kari == 0) a++;
    else{
    	while( kari != 0 ){
    		kari /= 10;
    		a++;
    		}
    	}
    
    for(i = 1 ;i < a ; i++) x *= 10;
    
    for(i=1 ; i<=keta ; i++){
    	if(i <= keta-a) temp_chr[i-1] = 0x20;
    	else{
    		if( num < 0 ){
    			num=-(num);
    			temp_chr[i-2]='-';
    		}
    		y=num/x;
    		temp_chr[i-1]=(0x30)|(char)y;
    		num=num-(y*x);
    		x/=10;
    	}
    }
    	
    temp_chr[i-1]='\0';
    i=0;
    while(temp_chr[i]!='\0'){
    sci_change_num(  temp_chr[i] );
    i++;
    }	
   
}
// ���l������+�]��
void sci_change_num(char num_1byte)
{
    num_1byte |= 0x30;		//�A�X�L�[�ϊ�
    sci_tx( num_1byte );
}
