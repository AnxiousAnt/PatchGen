#include "c:\programs\include\billstuff.h"
#define pi 3.14159265359
#define PI 3.14159265359

class riffWave{
public:
char cpId[4];          // Identifier for the file should always be RIFF
DWORD dwSize;          // The size of the entire file - the previous 4 bytes
char cpRiffId[4];      // RIFF identifier should be WAVE
char cpFormatId[4];    // Wave format Identifier should be fmt with a space at the end
DWORD dwFormatSize;    // Size of the format information should be 16 bytes
WORD wFormatTag;       // 1=PCM
WORD wChannels;        // Number of channels 1=mono 2=stereo
DWORD dwSampleRate;    // Sample rate (44100, 22050, 11025, 8000)
DWORD dwAvgBytesPerSec;// (Sample Rate * Channels *(bits/8))
WORD wBlockAlign;      // (Bits/8)*Channels)
WORD wBitsPerSample;   // (8 or 16)
char cpDataId[4];      // data specifier should be data
DWORD dwDataSize;      // Length of the Data
DWORD dwNumSamples;
BYTE *bWave;
short *wWave;

/*/Variables Needed to Initalize a Windows PCM sound File  
Channels
Sample Rate
Bits Per Sample
/*/
riffWave();
void loadWaveFile(char *fname);
void writeWaveFile(char *fname);
void createWaveHeader(WORD c, DWORD sr, WORD bps);
void createWaveSpace(DWORD l);
void createSineWave(WORD a, double f, DWORD l);
void oldhiPassFilter(double f);
void hiPassFilter(double f);
void loPassFilter(double f);
void notchFilter(double f,double f2);
void newNotchFilter(double f,double f2);
void bandPassFilter(double frequency, double bandwidth, BYTE scale);
void Filter(double frequency,double Q);
void loPass(double frequency,double Q);
void hiPass(double frequency,double Q);
void loShelf(double frequency,double S,double dbGain);
void hiShelf(double frequency,double S,double dbGain);
void Print();
~riffWave();
};
riffWave *mcWave;
riffWave *myWave;
riffWave *myOtherWave;
riffWave *source[6];
riffWave *control[6];
//////////////////////////////////////////////////////////////////////////
void main()
{
DWORD x;
DWORD numsamples;
DWORD i;
double gF1,gF2,gF3,gF4,GF5;
myWave=new riffWave;
printf("Initializing Wave Variables.\n");
for(x=0;x<8;x++)
{
source[x]=new riffWave;
control[x]=new riffWave;
}

//myWave->loadWaveFile("c:\\samples\\StiltnerRZX1\\voice\\greasevocode.wav");
//myWave->Print();
//myWave->createWaveHeader(1,44100,16);
printf("Loading control\n");
control[0]->loadWaveFile("c:\\samples\\StiltnerRZX1\\voice\\laser beams3.wav");
printf("Loading Source\n");
source[0]->loadWaveFile("c:\\samples\\StiltnerRZX1\\synth\\metal.wav");

numsamples=source[0]->dwNumSamples;

printf("Creating Headers.\n");
for(x=0;x<6;x++)
{
//initialize wave variables
source[x]->createWaveHeader(1,44100,16);
control[x]->createWaveHeader(1,44100,16);
}

printf("Creating Space.\n");
for(x=1;x<6;x++)
{
//allocate space
source[x]->createWaveSpace(numsamples);
control[x]->createWaveSpace(numsamples);
}

printf("Copying Source and control to bank arrays and filtering. \n");
for(x=1;x<6;x++)
{
for(i=0;i<numsamples;i++)
  {
   //copy source and control into wave array bank
  source[x]->wWave[i]=source[0]->wWave[i];
  control[x]->wWave[i]=control[0]->wWave[i];
  }
}
//filter out 
control[1]->loPass(200,.667);
control[2]->Filter(400,0.667);
control[3]->Filter(800,0.667);
control[4]->Filter(1600,0.667);
control[5]->hiPass(3200,.667);


source[1]->loPass(200,.667);
source[2]->Filter(400,0.667);
source[3]->Filter(800,0.667);
source[4]->Filter(1600,0.667);
source[5]->hiPass(3200,.667);
double G;

//for(i=0;i<numsamples;i++)
//  {
//this is a test to see what the filtered signals put out
//source[0]->wWave[i]=(source[1]->wWave[i]+source[2]->wWave[i]+source[3]->wWave[i]+source[4]->wWave[i]+source[5]->wWave[i])/5;
//control[0]->wWave[i]=(control[1]->wWave[i]+control[2]->wWave[i]+control[3]->wWave[i]+control[4]->wWave[i]+control[5]->wWave[i])/5;
//}
printf("Writing filtered output.\n");
source[0]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\source.wav");
source[1]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\source1.wav");
source[2]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\source2.wav");
source[3]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\source3.wav");
source[4]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\source4.wav");
source[5]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\source5.wav");

control[0]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\control.wav");
control[1]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\control1.wav");
control[2]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\control2.wav");
control[3]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\control3.wav");
control[4]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\control4.wav");
control[5]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\control5.wav");


printf("Rectifying filtered control.\n");
/////new rectify the filtered output
for(x=1;x<6;x++)
{
for(i=0;i<numsamples;i++)
  {
    control[x]->wWave[i]=abs(control[x]->wWave[i]);
  }
}


printf("amplifying filtered source by gain of filtered control.\n");
for(x=1;x<6;x++)
{

//filter the rectified output
//control[x]->loPass(20,.667);
for(i=0;i<numsamples;i++)
  {

   if(control[0]->wWave[i]==0){
      G=0;
     }else{
        G=abs(double(control[x]->wWave[i]/(control[0]->wWave[i])));
     }

//G=control[x]->wWave[i];
 
  source[x]->wWave[i]=(source[x]->wWave[i]*G);
  }
//source[x]->loPass(1600,.667);


}
/*/
printf("Refiltering source.\n");
source[1]->loPass(200,.667);
source[2]->loPass(200,.667);
source[3]->loPass(200,.667);
source[4]->loPass(200,.667);
source[5]->loPass(200,.667);
/*/
printf("Mixing Output.\n");
for(i=0;i<numsamples;i++)
  {
//  source[0]->wWave[i]=(source[1]->wWave[i]+source[2]->wWave[i]+source[3]->wWave[i]+source[4]->wWave[i]+source[5]->wWave[i]+source[6]->wWave[i]+source[7]->wWave[i])/7;
source[0]->wWave[i]=short(double(source[1]->wWave[i]+source[2]->wWave[i]+source[3]->wWave[i]+source[4]->wWave[i]+source[5]->wWave[i])/5);
//source[0]->wWave[i]=(source[1]->wWave[i]+source[5]->wWave[i])/2;
  }

//myWave->newNotchFilter(400,400);


printf("Writing new rectified filtered amplified output.\n");
control[0]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vcontrol.wav");
control[1]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vcontrol1.wav");
control[2]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vcontrol2.wav");
control[3]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vcontrol3.wav");
control[4]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vcontrol4.wav");
control[5]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vcontrol5.wav");

source[0]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vsource.wav");
source[1]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vsource1.wav");
source[2]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vsource2.wav");
source[3]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vsource3.wav");
source[4]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vsource4.wav");
source[5]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\vsource5.wav");

printf("writing vocoded waveform.\n");
source[0]->writeWaveFile("c:\\samples\\StiltnerRZX1\\voice\\greasevocoded.wav");

delete(myWave);


}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
riffWave::riffWave()
{
bWave=NULL;
wWave=NULL;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
riffWave::~riffWave()
{
if(bWave!=NULL)delete bWave;
if(wWave!=NULL)delete bWave;
}
//////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
void riffWave::createWaveHeader(WORD c, DWORD sr, WORD bps)
{
wChannels=c;      // Number of channels 1=mono 2=stereo
dwSampleRate=sr;    // Sample rate (44100, 22050, 11025, 8000)
wBitsPerSample=bps; // (8 or 16)

if(wChannels!=1||wBitsPerSample!=16){
printf("stereo and 8 bit Not implemented");
return;
}

cpId[0]='R';          // Identifier for the file should always be RIFF
cpId[1]='I';
cpId[2]='F';
cpId[3]='F';
cpRiffId[0]='W';      // RIFF identifier should be WAVE
cpRiffId[1]='A';
cpRiffId[2]='V';
cpRiffId[3]='E';
cpFormatId[0]='f';    // Wave format Identifier should be fmt with a space at the end
cpFormatId[1]='m';
cpFormatId[2]='t';
cpFormatId[3]=' ';
cpDataId[0]='d';      // data specifier should be data
cpDataId[1]='a';
cpDataId[2]='t';
cpDataId[3]='a';
dwFormatSize=16;    // Size of the format information should be 16 bytes
wFormatTag=1;     // 1=PCM
dwAvgBytesPerSec=dwSampleRate*wChannels*(wBitsPerSample / 8);// (Sample Rate * Channels *(bits/8))
wBlockAlign=(wBitsPerSample / 8) * wChannels;    // (Bits/8)*Channels)
//dwSize;          // The size of the entire file - the previous 4 bytes
//DWORD dwDataSize;      // Length of the Data
//char *cpData;

}
////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
void riffWave::createWaveSpace(DWORD l)
{
dwNumSamples=l;
dwDataSize=l*2;
wWave=new short[l];
}
//////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
void riffWave::bandPassFilter(double frequency,double bandwidth, BYTE scale)
{
/*
 * Sound Tools Bandpass effect file.
 *
 * Algorithm:  2nd order recursive filter.
 * Formula stolen from MUSIC56K, a toolkit of 56000 assembler stuff.
 * Quote:
 *   This is a 2nd order recursive band pass filter of the form.                
 *   y(n)= a * x(n) - b * y(n-1) - c * y(n-2)   
 *   where :    
 *        x(n) = "IN"           
 *        "OUT" = y(n)          
 *        c = EXP(-2*pi*cBW/S_RATE)             
 *        b = -4*c/(1+c)*COS(2*pi*cCF/S_RATE)   
 *   if cSCL=2 (i.e. noise input)               
 *        a = SQT(((1+c)*(1+c)-b*b)*(1-c)/(1+c))                
 *   else       
 *        a = SQT(1-b*b/(4*c))*(1-c)            
 *   endif      
 *   note :     cCF is the center frequency in Hertz            
 *        cBW is the band width in Hertz        
 *        cSCL is a scale factor, use 1 for pitched sounds      
 *   use 2 for noise.           
 */
double a,b,c;
double cf,bw;
double y,y1,y2;
DWORD x,n;
BYTE scl;

cf=frequency;
bw=bandwidth;
c=exp(-6.28318530718*bw/dwSampleRate);
b=-4*c/(1+c)*cos(6.28318530718*cf/dwSampleRate);
if(scl==2){
a=sqrt(((1+c)*(1+c)-b*b)*(1-c)/(1+c));
}else{
a=sqrt(1-b*b/(4*c))*(1-c);
}

y=0;
y1=0;
y2=0;
//y(n)= a * x(n) - b * y(n-1) - c * y(n-2)   
for(n=0;n<dwNumSamples;n++)
{
x=wWave[n];
y=(a*x-b*y1)-c*y2;
y2=y1;
y1=y;
wWave[n]=y;
}

}

////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
void riffWave::newNotchFilter(double f,double f2)
{
oldhiPassFilter(f);
loPassFilter(f2);
}
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
void riffWave::oldhiPassFilter(double f)
{
//1st order filter
double T;
double RC;
double out;
double oldin;
double oldout;
double A,B,C;
DWORD x;
/*/output[N] = B * (output[N-1] - input[N-1] + input[N])

 	A = 2.0 * pi * frequency/samplerate
 	B = exp(-A / samplerate)

C=exp(-((2pi*frequency/samplerate)/samplerate)
/*/
A=6.28318530718*f/dwSampleRate;
B=exp(-A/dwSampleRate);
//C=exp(-(6.28318530718*f));
oldout=0.0;
oldin=0.0;
for(x=0; x<dwNumSamples; x++)
{
 out=B*(oldout - oldin + wWave[x]);
 out=0.8*out;
 oldout=out;
 oldin=wWave[x];
 wWave[x]=out;
}
}
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
void riffWave::loPassFilter(double f)
{
//Infinite Impulse Response Filter
double T;
double RC;
double y;
double oldy;
DWORD x;
//This function implements an electronic RC Low Pass Filter 
T=1.0/dwSampleRate;
//RC=sqrt(3)/(2pi*f)
RC=1.73205080757/(6.28318530718*f);
oldy=0;
for(x=0; x<dwNumSamples; x++)
   {
    y=((T/(T+RC)) * wWave[x])+((RC/(T+RC)) * oldy);
    oldy=y;
    //hi=wWave[x]-y;//use this to get the high frequencies 
       wWave[x]=y;
   }
}
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
void riffWave::hiPassFilter(double f)
{
//Infinite Impulse Response Filter
double T;
double RC;
double y;
double oldy;
DWORD x;
//This function simulates the RC Low Pass Filter
//then subtracts those frequencies from the original
//waveform to get the high frequencies
T=1.0/dwSampleRate;
//RC=sqrt(3)/(2pi*f)
RC=1.73205080757/(6.28318530718*f);
oldy=0;
for(x=0; x<dwNumSamples; x++)
   {
    y=((T/(T+RC)) * wWave[x])+((RC/(T+RC)) * oldy);
    oldy=y;
    y=wWave[x]-y;
    wWave[x]=y;
    }
}
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
void riffWave::notchFilter(double f,double f2)
{
double T;
double RC;
double RC2;
double lo;
double hi;
double notchhi;
double notchlo;
double oldlo;
double oldhi;
double out;
DWORD x;
T=1.0/dwSampleRate;
//RC=sqrt(3)/(2pi*f)
RC=1.73205080757/(6.28318530718*f);
RC2=1.73205080757/(6.28318530718*f2);
oldlo=0;
oldhi=0;
for(x=0; x<dwNumSamples; x++)
{
lo=((T/(T+RC)) * wWave[x])+((RC/(T+RC)) * oldlo);
oldlo=lo;

hi=((T/(T+RC)) * wWave[x])+((RC2/(T+RC2)) * oldhi);
oldhi=hi;
hi=wWave[x]-hi;//use this for a notch filter

out=(hi+lo)/2;
out=wWave[x]-out;

//notchhi=wWave[x]-hi;
//notchlo=wWave[x]-lo;
//out=notchhi+notchlo;
wWave[x]=out;

}
/*/
var RC = Math.sqrt(3) / (2.0 * Math.PI * fc)
var T = 1.0 / sr
var c0 = T / (T + RC)
form.c0.value = c0
form.d1.value = 1.0 - c0
/*/
}
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
void riffWave::createSineWave(WORD a, double f, DWORD l)
{
double Frequency;
dwNumSamples=l;
short y;
DWORD x;
DWORD i;
char strWave[256];
char strL[256];
double z,m,n;

dwSize=(l*2)+40;
dwDataSize=l*2;


Frequency=f;
wWave=new short[l];

x=0;
i=0;
ltoa((long)l,strL,10);
printf("l=");
printf(strL);
printf("\n");
//n=Frequency/(2*3.1416)/dwSampleRate;
n=Frequency*(2*3.14159265359);
n=n/dwSampleRate;

for(i=0; i < l; i++)
{ 
m=n*i;
z=a*sin(m);
y=z+0;
y=short(z+0);
//for sin that uses degrees this might work see sine.txt
//y = a * (sin(((Frequency*360)/dwSampleRate)* i));
wWave[i]=y;
x++;
}
/*/
for(i = 0; i < l; i++)
{
ltoa((long)wWave[i],strWave,10);
printf(strWave);
}/*/
}
////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void riffWave::loadWaveFile(char *fname) 
{ 
FILE *fp; 
char strId[5];
char strRiffId[5];
char strFormatId[5];
char strDataId[5];

strId[4]='\0';
strRiffId[4]='\0';
strFormatId[4]='\0';
strDataId[4]='\0';

fp = fopen(fname,"rb"); 
if(fp) 
{ 

 fread(cpId, sizeof(BYTE), 4, fp); //read in first four bytes 
 strId[0]=cpId[0];strId[1]=cpId[1];
 strId[2]=cpId[2];strId[3]=cpId[3];
 if(!strcmp(strId, "RIFF")) 
    {  //we had 'RIFF' let's continue 
     fread(&dwSize, sizeof(DWORD), 1, fp); //read in 32bit size value 
     fread(cpRiffId, sizeof(BYTE), 4, fp); //read in 4 byte string now 
      strRiffId[0]=cpRiffId[0];strRiffId[1]=cpRiffId[1];
      strRiffId[2]=cpRiffId[2];strRiffId[3]=cpRiffId[3];
     if(!strcmp(strRiffId,"WAVE")) 
       { //this is probably a wave file since it contained "WAVE" 
        fread(cpFormatId, sizeof(BYTE), 4, fp); //read in 4 bytes "fmt "; 
         strFormatId[0]=cpFormatId[0];strFormatId[1]=cpFormatId[1];
         strFormatId[2]=cpFormatId[2];strFormatId[3]=cpFormatId[3];
        fread(&dwFormatSize, sizeof(DWORD),1,fp); 
        fread(&wFormatTag, sizeof(WORD), 1, fp); //check mmreg.h (i think?) for other possible format tags like ADPCM 
        fread(&wChannels, sizeof(WORD),1,fp); //1 mono, 2 stereo 
        fread(&dwSampleRate, sizeof(DWORD), 1, fp); //like 44100, 22050, etc... 
        fread(&dwAvgBytesPerSec, sizeof(DWORD), 1, fp); //probably won't need this 
        fread(&wBlockAlign, sizeof(WORD), 1, fp); //probably won't need this 
        fread(&wBitsPerSample, sizeof(WORD), 1, fp); //8 bit or 16 bit file? 
        fread(cpDataId, sizeof(BYTE), 4, fp); //read in 'data' 
         strDataId[0]=cpDataId[0];strDataId[1]=cpDataId[1];
         strDataId[2]=cpDataId[2];strDataId[3]=cpDataId[3];
        fread(&dwDataSize, sizeof(DWORD), 1, fp); //how many bytes of sound data we have 

if(wBitsPerSample==8)
{
bWave=new BYTE[dwDataSize];
dwNumSamples=dwDataSize;
}

if(wBitsPerSample==16)
{
wWave=new short[dwDataSize/2];
dwNumSamples=dwDataSize/2;
}

if(wBitsPerSample==8){
         fread(bWave, sizeof(BYTE), dwDataSize, fp); //read in our whole sound data chunk 
}
if(wBitsPerSample==16){
         fread(wWave, sizeof(short), dwNumSamples, fp); //read in our whole sound data chunk 
}
         fclose(fp);
        }else printf("Error: RIFF file but not a wave file\n"); 
     }else printf("Error: not a RIFF file\n"); 
}
if(wChannels!=1||wBitsPerSample!=16){
printf("stereo and 8 bit Not implemented");
return;
}

}
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void riffWave::writeWaveFile(char *fname) 
{ 
FILE *fp; 
fp = fopen(fname,"wb+"); 

if(wChannels!=1||wBitsPerSample!=16){
printf("stereo and 8 bit Not implemented");
return;
}

if(fp) 
{ 
 fwrite(cpId, sizeof(BYTE), 4, fp); //read in first four bytes 
 fwrite(&dwSize, sizeof(DWORD), 1, fp); //read in 32bit size value 
 fwrite(cpRiffId, sizeof(BYTE), 4, fp); //read in 4 byte string now 
 fwrite(cpFormatId, sizeof(BYTE), 4, fp); //read in 4 bytes "fmt "; 
 fwrite(&dwFormatSize, sizeof(DWORD),1,fp); 
 fwrite(&wFormatTag, sizeof(WORD), 1, fp); //check mmreg.h (i think?) for other possible format tags like ADPCM 
 fwrite(&wChannels, sizeof(WORD),1,fp); //1 mono, 2 stereo 
 fwrite(&dwSampleRate, sizeof(DWORD), 1, fp); //like 44100, 22050, etc... 
 fwrite(&dwAvgBytesPerSec, sizeof(DWORD), 1, fp); //probably won't need this 
 fwrite(&wBlockAlign, sizeof(WORD), 1, fp); //probably won't need this 
 fwrite(&wBitsPerSample, sizeof(WORD), 1, fp); //8 bit or 16 bit file? 
 fwrite(cpDataId, sizeof(BYTE), 4, fp); //read in 'data' 
 fwrite(&dwDataSize, sizeof(DWORD), 1, fp); //how many bytes of sound data we have 
//if(wBitsPerSample==8)
//{
//bWave=new BYTE[dwDataSize];
//}
//if(wBitsPerSample==16)
//{
//wWave=new short[dwDataSize/2];
//}

if(wBitsPerSample==8){
         fwrite(bWave, sizeof(BYTE), dwDataSize, fp); //read in our whole sound data chunk 
}
if(wBitsPerSample==16){
         fwrite(wWave, sizeof(short), dwNumSamples, fp); //read in our whole sound data chunk 
}

 fclose(fp);
}else printf("/ncannot open output file/n");
}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
void riffWave::Print(){
char strId[5];
char strSize[256];
char strRiffId[5];
char strFormatId[5];
char strFormatSize[256];
char strFormatTag[256];
char strChannels[256];
char strSampleRate[256];
char strAvgBytesPerSec[256];
char strBlockAlign[256];
char strBitsPerSample[256]; //our 16 values 
char strDataId[5];
char strDataSize[256];

strId[4]='\0';
strRiffId[4]='\0';
strFormatId[4]='\0';
strDataId[4]='\0';


strId[0]=cpId[0];strId[1]=cpId[1];
strId[2]=cpId[2];strId[3]=cpId[3];
strRiffId[0]=cpRiffId[0];strRiffId[1]=cpRiffId[1];
strRiffId[2]=cpRiffId[2];strRiffId[3]=cpRiffId[3];
strFormatId[0]=cpFormatId[0];strFormatId[1]=cpFormatId[1];
strFormatId[2]=cpFormatId[2];strFormatId[3]=cpFormatId[3];
strDataId[0]=cpDataId[0];strDataId[1]=cpDataId[1];
strDataId[2]=cpDataId[2];strDataId[3]=cpDataId[3];


ltoa((long)dwSize,strSize,10);
ltoa((long)dwFormatSize,strFormatSize,10);
ltoa((long)wFormatTag,strFormatTag,10);
ltoa((long)wChannels,strChannels,10);
ltoa((long)dwSampleRate,strSampleRate,10);
ltoa((long)dwAvgBytesPerSec,strAvgBytesPerSec,10);
ltoa((long)wBlockAlign,strBlockAlign,10);
ltoa((long)wBitsPerSample,strBitsPerSample,10);
ltoa((long)dwDataSize,strDataSize,10);

printf("Size = ");
printf(strSize);
printf("\n");

printf("Id = ");
printf(strId);
printf("\n");

printf("RiffId = ");
printf(strRiffId);
printf("\n");

printf("FormatId = ");
printf(strFormatId);
printf("\n");


printf("Format Size = ");
printf(strFormatSize);
printf("\n");

printf("Format Tag = ");
printf(strFormatTag);
printf("\n");

printf("Channels = ");
printf(strChannels);
printf("\n");

printf("Sample Rate = ");
printf(strSampleRate);
printf("\n");

printf("Avg Bytes Per Sec = ");
printf(strAvgBytesPerSec);
printf("\n");

printf("Block Aalign = ");
printf(strBlockAlign);
printf("\n");

printf("Bits Per Sample = ");
printf(strBitsPerSample);
printf("\n");

printf("Data Id = ");
printf(strDataId);
printf("\n");


printf("Data Size = ");
printf(strDataSize);
printf("\n");

}
//////////////////////////////////////////////////////////////////////////////

void riffWave::Filter(double frequency,double Q)
{
DWORD n,n1,n2;
n1=0;
n2=0;
short *y;
double a0,a1,a2,b0,b1,b2;
double omega,sn,cs,alpha;
y=new short[dwNumSamples];

omega = 2*PI*frequency/dwSampleRate;
sn    = sin(omega);
cs    = cos(omega);
alpha = sn/(2*Q);

b0 =   alpha;
b1 =   0;
b2 =  -alpha;
a0 =   1 + alpha;
a1 =  -2*cs;
a2 =   1 - alpha;


n1=0;
n2=0;
y[0]=0;
y[1]=0;
y[2]=0;

for(n=2;n<dwNumSamples;n++)
{
y[n] = (b0/a0)*wWave[n] + (b1/a0)*wWave[n-1] + (b2/a0)*wWave[n-2] - (a1/a0)*y[n-1] - (a2/a0)*y[n-2];
}

for(n=0;n<dwNumSamples;n++)
{
wWave[n]=y[n];

}
delete y;
}


void riffWave::loPass(double frequency,double Q)
{
DWORD n,n1,n2;
n1=0;
n2=0;
short *y;
double a0,a1,a2,b0,b1,b2;
double omega,sn,cs,alpha;
y=new short[dwNumSamples];

omega = 2*PI*frequency/dwSampleRate;
sn    = sin(omega);
cs    = cos(omega);
alpha = sn/(2*Q);

b0 =  (1 - cs)/2;
b1 =   1 - cs;
b2 =  (1 - cs)/2;
a0 =   1 + alpha;
a1 =  -2*cs;
a2 =   1 - alpha;


n1=0;
n2=0;
y[0]=0;
y[1]=0;
y[2]=0;

for(n=2;n<dwNumSamples;n++)
{
y[n] = (b0/a0)*wWave[n] + (b1/a0)*wWave[n-1] + (b2/a0)*wWave[n-2] - (a1/a0)*y[n-1] - (a2/a0)*y[n-2];
}

for(n=0;n<dwNumSamples;n++)
{
wWave[n]=y[n];

}
delete y;
}

void riffWave::hiPass(double frequency,double Q)
{
DWORD n,n1,n2;
n1=0;
n2=0;
short *y;
double a0,a1,a2,b0,b1,b2;
double omega,sn,cs,alpha;
y=new short[dwNumSamples];

omega = 2*PI*frequency/dwSampleRate;
sn    = sin(omega);
cs    = cos(omega);
alpha = sn/(2*Q);


b0 =  (1 + cs)/2;
b1 = -(1 + cs);
b2 =  (1 + cs)/2;
a0 =   1 + alpha;
a1 =  -2*cs;
a2 =   1 - alpha;

n1=0;
n2=0;
y[0]=0;
y[1]=0;
y[2]=0;

for(n=2;n<dwNumSamples;n++)
{
y[n] = (b0/a0)*wWave[n] + (b1/a0)*wWave[n-1] + (b2/a0)*wWave[n-2] - (a1/a0)*y[n-1] - (a2/a0)*y[n-2];
}

for(n=0;n<dwNumSamples;n++)
{
wWave[n]=y[n];

}
delete y;
}






void riffWave::loShelf(double frequency,double S,double dbGain)
{
DWORD n,n1,n2;
n1=0;
n2=0;
short *y;
double a0,a1,a2,b0,b1,b2;
double omega,sn,cs,alpha,beta,A;
y=new short[dwNumSamples];

A     = sqrt( pow(10,dbGain/20));

beta  = sqrt( (pow(A,2) + 1)/S - pow((A-1),2) );

omega = 2*PI*frequency/dwSampleRate;
sn    = sin(omega);
cs    = cos(omega);



b0 =    A*( (A+1) - (A-1)*cs + beta*sn );
b1 =  2*A*( (A-1) - (A+1)*cs           );
b2 =    A*( (A+1) - (A-1)*cs - beta*sn );
a0 =        (A+1) + (A-1)*cs + beta*sn;
a1 =   -2*( (A-1) + (A+1)*cs           );
a2 =        (A+1) + (A-1)*cs - beta*sn;


n1=0;
n2=0;
y[0]=0;
y[1]=0;
y[2]=0;

    


for(n=2;n<dwNumSamples;n++)
{
y[n] = (b0/a0)*wWave[n] + (b1/a0)*wWave[n-1] + (b2/a0)*wWave[n-2] - (a1/a0)*y[n-1] - (a2/a0)*y[n-2];
}

for(n=0;n<dwNumSamples;n++)
{
wWave[n]=y[n];

}
delete y;
}

void riffWave::hiShelf(double frequency,double S,double dbGain)
{
DWORD n,n1,n2;
n1=0;
n2=0;
short *y;
double a0,a1,a2,b0,b1,b2;
double omega,sn,cs,alpha,beta,A;
y=new short[dwNumSamples];

A     = sqrt( pow(10,dbGain/20));

beta  = sqrt( (pow(A,2) + 1)/S - pow((A-1),2) );

omega = 2*PI*frequency/dwSampleRate;
sn    = sin(omega);
cs    = cos(omega);



b0 =    A*( (A+1) + (A-1)*cs + beta*sn );
b1 = -2*A*( (A-1) + (A+1)*cs           );
b2 =    A*( (A+1) + (A-1)*cs - beta*sn );
a0 =        (A+1) - (A-1)*cs + beta*sn;
a1 =    2*( (A-1) - (A+1)*cs           );
a2 =        (A+1) - (A-1)*cs - beta*sn;


n1=0;
n2=0;
y[0]=0;
y[1]=0;
y[2]=0;

    


for(n=2;n<dwNumSamples;n++)
{
y[n] = (b0/a0)*wWave[n] + (b1/a0)*wWave[n-1] + (b2/a0)*wWave[n-2] - (a1/a0)*y[n-1] - (a2/a0)*y[n-2];
}

for(n=0;n<dwNumSamples;n++)
{
wWave[n]=y[n];

}
delete y;
}



/*/
now, given:

    frequency (wherever it's happenin', man.  "center" frequency or 
        "corner" (-3 dB) frequency, or shelf midpoint frequency, 
        depending on which filter type)
    
    dBgain (used only for peaking and shelving filters)

    bandwidth in octaves (between -3 dB frequencies for BPF and notch
        or between midpoint (dBgain/2) gain frequencies for peaking )

     _or_ Q (the EE kinda definition)

     _or_ S, a "shelf slope" parameter.  when S = 1, the shelf slope is 
        as steep as you can get it and remain monotonically increasing 
        or decreasing gain with frequency.


        
first compute a few intermediate variables:

    A     = sqrt[ 10^(dBgain/20) ]   = 10^(dBgain/40)    (for for
peaking and shelving EQ
filters only)

    omega = 2*PI*frequency/sample_rate
    
    sn    = sin(omega)
    cs    = cos(omega)

    alpha = sn/(2*Q)
          = sn*sinh[ ln(2)/2 * bandwidth * omega/sn ]     (if bandwidth
is specified instead of
Q)

    beta  = sqrt[ (A^2 + 1)/S - (A-1)^2 ]   (for shelving EQ filters
only)


then compute the coefs for whichever filter type you want:


  the analog prototypes are shown for normalized frequency.
  the bilinear transform substitutes
  
                1          1 - z^-1
  s  <-  -------------- * ----------
          tan(omega/2)     1 + z^-1



LPF:            H(s) = 1 / (s^2 + s/Q + 1)

                b0 =  (1 - cs)/2
                b1 =   1 - cs
                b2 =  (1 - cs)/2
                a0 =   1 + alpha
                a1 =  -2*cs
                a2 =   1 - alpha



HPF:            H(s) = s^2 / (s^2 + s/Q + 1)

                b0 =  (1 + cs)/2
                b1 = -(1 + cs)
                b2 =  (1 + cs)/2
                a0 =   1 + alpha
                a1 =  -2*cs
                a2 =   1 - alpha



BPF:            H(s) = (s/Q) / (s^2 + s/Q + 1)

                b0 =   alpha
                b1 =   0
                b2 =  -alpha
                a0 =   1 + alpha
                a1 =  -2*cs
                a2 =   1 - alpha



notch:          H(s) = (s^2 + 1) / (s^2 + s/Q + 1)

                b0 =   1
                b1 =  -2*cs
                b2 =   1
                a0 =   1 + alpha
                a1 =  -2*cs
                a2 =   1 - alpha



peakingEQ:      H(s) = (s^2 + s*(A/Q) + 1) / (s^2 + s/(A*Q) + 1)

                b0 =   1 + alpha*A
                b1 =  -2*cs
                b2 =   1 - alpha*A
                a0 =   1 + alpha/A
                a1 =  -2*cs
                a2 =   1 - alpha/A



lowShelf:       H(s) = A * (A + beta*s + s^2) / (1 + beta*s + A*s^2)

                b0 =    A*[ (A+1) - (A-1)*cs + beta*sn ]
                b1 =  2*A*[ (A-1) - (A+1)*cs           ]
                b2 =    A*[ (A+1) - (A-1)*cs - beta*sn ]
                a0 =        (A+1) + (A-1)*cs + beta*sn
                a1 =   -2*[ (A-1) + (A+1)*cs           ]
                a2 =        (A+1) + (A-1)*cs - beta*sn



highShelf:      H(s) = A * (1 + beta*s + A*s^2) / (A + beta*s + s^2)

                b0 =    A*[ (A+1) + (A-1)*cs + beta*sn ]
                b1 = -2*A*[ (A-1) + (A+1)*cs           ]
                b2 =    A*[ (A+1) + (A-1)*cs - beta*sn ]
                a0 =        (A+1) - (A-1)*cs + beta*sn
                a1 =    2*[ (A-1) - (A+1)*cs           ]
                a2 =        (A+1) - (A-1)*cs - beta*sn

/*/



