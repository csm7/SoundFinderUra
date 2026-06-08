
#include "pch.h"

#include "wavlib.h"
#include "decodb.h"
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

// -----------------------------------------------------------------------------

#define FALSE 0
#define TRUE 1

/*
int CalcPath(char* path)
{
	int		i = 0;
	int		j;
	while (path[i] != 0x0)
	{
		if (path[i] == '/') path[i] = '\\';
		if ((i > 1) && (path[i] == ':')) path[i] = '\\';
		while ((path[i] == '\\') && (path[i + 1] == '\\'))
		{
			j = i + 1;
			while (path[j] != 0x0)
			{
				path[j - 1] = path[j];
				j++;
			}
			path[j - 1] = 0x0;
		};
		i++;
	};
	if ((i != 0) && (path[i - 1] != '\\')) strcat_s(path,1, "\\");
	return 0;
};
*/
/*
int MakeDir(char* path)				//
{
	size_t i = 0, j = 0;

	while (path[i] != 0x0)
		i++;
	if ((i != 0) && (path[i - 1] == '\\'))
		path[i - 1] = 0;
	if (CreateDirectoryA(path, 0))
	{
		if (path[i - 1] == 0)
			path[i - 1] = '\\';
		return 0;
	}
	while (path[j] != 0x0)
	{
		j++;
		if (path[j] == '\\')
		{
			path[j] = 0;
			CreateDirectoryA(path, 0);
			path[j] = '\\';
		}
	}
	if (CreateDirectoryA(path, 0))
	{
		if (path[i - 1] == 0) path[i - 1] = '\\';
		return 0;
	}
	if (path[i - 1] == 0)
		path[i - 1] = '\\';

	return 1;
};
*/
// --------------------------------------------
uchar	Compand_Alaw (short q)
{
short	z=0,i,j,p=0;

	if(q&0x8000) { z=0x80; q=-q; }
	i=0; while(1)
	{
	  p+=16; if(q<=alaw_[p]) break;
	  if(i==7) break; else i++;
	} p-=16;
	j=0; while(1)
	{
	  if(q<alaw_[++p]) break;
	  if(j==15) break; else j++;
	} return (z|(i<<4)|j)^0xd5;
}
uchar	Compand_Mlaw (short q)
{
	return q>>8;
}
//------ A-law Interface -------------------------------------------------------
short	Alaw2Short(uchar t)
{
	if(t&0x80) return  alaw[(t^0xd5)&0x7f];
	else       return -alaw[(t^0xd5)&0x7f];
}
float	Alaw2float(uchar t) {return (float)Alaw2Short(t);}

uchar	Short2Alaw(short t)
{
	return Compand_Alaw(t);
}
uchar	Float2Alaw(float t) {return Short2Alaw((short)t);}
//------ m-law Interface -------------------------------------------------------
short	Mlaw2Short(uchar t)
{
	if(t&0x80) return  mlaw[(t^0xfe)&0x7f];
	else       return -mlaw[(t^0xfe)&0x7f];
}
float	Mlaw2float(uchar t) {return (float)Mlaw2Short(t);}

uchar	Short2Mlaw(short t)
{
	return Compand_Mlaw(t);
}
uchar	Float2Mlaw(float t) {return Short2Mlaw((short)t);}
// -----------------------------------------------------------------------------
inline short	IKM2Short(uchar t)
{
	if(t&1) return  alaw[(rvt[t]^0xd5)&0x7f];
	else    return -alaw[(rvt[t]^0xd5)&0x7f];
}
// -----------------------------------------------------------------------------
// Библиотека для работы с WAV-файлами. Ограниечения:
// 1) буфера для записи/чтения используются для конвертации
// 2) поле data должно быть последним при работе с VeryFast
// 3) IKM-конвертор работает только на чтение SetFormat(IKM);
// 4) 8bit-конвертор работает только на чтение SetFormat(PCM8);

int    FSample[mFS]      ={8000,11025,22050,44100};
struct fmt_  fmt_Default ={1,1,8000,16000,2,16};
//struct fmt_  fmt_Default = { 1,1,8000,16000,2,16,0 };
struct fact_ fact_Default={8000};
float  freq_Default=0.;
char   LIST_Default[256]={0};

Tag::Tag(char *s, int l)
{
	memcpy_s(name,4,s,4);  length=l;
	Buf=(char *)malloc(length);
}

Tag::~Tag()
{
	free(Buf);
}
//------------------------------------------------------------------------------
FileWAV::FileWAV(uint dBuf)
{
	RIFF=new Tag((char*)"RIFF",4);
	memcpy(RIFF->Buf, "WAVE", RIFF->length);

	fmt =new Tag((char*)"fmt ",sizeof(fmt_));
	memcpy(fmt->Buf, &fmt_Default, fmt->length);
/*
	fact =new Tag((char*)"fact",sizeof(fact_));
	memcpy(fact->Buf, &fact_Default, fact->length);

	freq=new Tag((char*)"freq",4);
	//memset(freq->Buf, 0, freq->length);
	memcpy(freq->Buf, &freq_Default, freq->length);

	LIST=new Tag((char*)"LIST",0x100);
	//memset(LIST->Buf, 0, LIST->length);
	memcpy(LIST->Buf, &LIST_Default, LIST->length);
*/
	data=new Tag((char*)"data",4*dBuf);
	memset(data->Buf, 0, data->length);

	stBuf=0x10000; tBuf=(PUCHAR)malloc(4*stBuf);

	file[0]=0; mode=0; handle=0;
	ptr=0; datalen=0; DataLen64=0;
	FlagUpdate=FALSE;
}

FileWAV::~FileWAV()
{
	free(tBuf);

	delete RIFF;
	delete fmt;
//	delete fact;
//	delete freq;
//	delete LIST;
	delete data;
}


int	FileWAV::Create(int Number)
{
	CalcNameData(file, Number); //	strcpy(file, f);
	strcat(file, ".wav");
	_fmode = O_BINARY;
	if (handle < 1) handle = creat(file, S_IWRITE);
	mode = WROnly; ptr = 0; datalen = 0; DataLen64 = 0;
	FlagUpdate = TRUE;
	return handle;
}
// создание нового файла
int	FileWAV::Create(char *f)
{
	strcpy(file,f); _fmode=O_BINARY;
	if(handle<1) handle=creat(file, S_IWRITE);
	mode=WROnly; ptr=0; datalen=0; DataLen64=0;
	FlagUpdate= TRUE;
	return handle;
}

//
int	FileWAV::Create2(char* cat,char* ext)
{
int	num=0; char t_buf[80]="",tn[9];
	while (1)
	{
	  strcpy(t_buf,cat);
	  CalcPath(t_buf);
  	  MakeDir(t_buf);
	  strcat(t_buf,CalcNameData(tn,num));
	  strcat(t_buf,ext); _fmode=O_BINARY;
	  handle=_open(t_buf,O_CREAT|O_EXCL|O_BINARY|O_RDWR);
//	  if (handle!=-1) SetFileAttributes(t_buf,FILE_ATTRIBUTE_HIDDEN);

//	  handle=creatnew(t_buf, FA_HIDDEN);
	  if(handle!=-1) break; else num++;
	  
	  if(num==1000) break; // gluk
	}
	return Create(t_buf);
}

int	FileWAV::Create3(char* cat,char* name,char* ext)
{
	int	num=0;
	char t_buf[256] = "";
	while (1)
	{
		strcpy(t_buf,cat);
		//CalcPath(t_buf);
		MakeDir(t_buf);
		strcat(t_buf,name);
		strcat(t_buf,ext); _fmode=O_BINARY;
		handle = _open(t_buf, O_CREAT | O_EXCL | O_BINARY | O_RDWR);	// _O_CREAT | _O_BINARY | _O_WRONLY, _S_IREAD | _S_IWRITE);
//		if (handle!=-1) SetFileAttributes(t_buf,FILE_ATTRIBUTE_HIDDEN);

		//	  handle=creatnew(t_buf, FA_HIDDEN);
		if(handle!=-1) break; else num++;

		if(num==1000) break; // gluk
	}
	return Create(t_buf);
}

//
int	FileWAV::Create4(char* cat, char* ext, int Number)
{
	int	num = 0; char t_buf[80] = "", tn[12];
	strcpy(t_buf, cat);
	CalcPath(t_buf);
	MakeDir(t_buf);
	strcat(t_buf, CalcNameData(tn, Number));
	strcat(t_buf, ext); _fmode = O_BINARY;
	handle = _open(t_buf, O_CREAT | O_EXCL | O_BINARY | O_RDWR);
	//	  if (handle!=-1) SetFileAttributes(t_buf,FILE_ATTRIBUTE_HIDDEN);

	//	  handle=creatnew(t_buf, FA_HIDDEN);
	return Create(t_buf);
}

// открытие существующего файла
int	FileWAV::Open(char *f, int m)
{
int	ret;
	strcpy_s(file,f); mode=m+1;
//	_fmode=O_BINARY;
	switch(mode)
	{
	  case RDOnly: _sopen_s(&handle, file, O_RDONLY | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE); break;
	  case WROnly: _sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE); break;
	  case RDWR  : _sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);   break;

	}
	if(handle==-1)
	{
		mode=0; 
		return handle;
	}

	DataLen64=_lseeki64(handle,0, SEEK_SET);		// SEEK_END ????
	ret=ReadFromFile(); // чтение из файла (datalen=...)
	ptr=0; FlagUpdate=FALSE;
	return ret;
}
// сохранение WAV-структуры в файле
int	FileWAV::WriteToFile(void)
{
int	ret=0;
	if (mode&WROnly) if (FlagUpdate)
	{
	  _lseek(handle,0L,SEEK_SET);
	  RIFF->length=4+
		       8+fmt ->length+
//			   8+fact->length+
//		       8+freq->length+
//		       8+LIST->length+
		       8+data->length;

	  ret|=_write(handle, RIFF->name, 8);
	  ret|=_write(handle, RIFF->Buf, 4);

	  ret|=_write(handle, fmt->name, 8);
	  ret|=_write(handle, fmt->Buf, fmt->length);
/*
	  ret|=_write(handle, fact->name, 8);
	  ret|=_write(handle, fact->Buf, fact->length);

	  freq->length+=0x108;
	  ret|=_write(handle, freq->name, 8);
	  freq->length-=0x108;
	  ret|=_write(handle, freq->Buf, freq->length);

	  ret|=_write(handle, LIST->name, 8);
	  ret|=_write(handle, LIST->Buf, LIST->length);
*/
	  ret|=_write(handle, data->name, 8);
	  ret|=_write(handle, data->Buf, datalen);

	  FlagUpdate=FALSE;
	}
	return ret;
}
// чтение WAV-структуры из файле
int	FileWAV::ReadFromFile(void)
{
	int	ret=0,t1,t2,*len;
	char	buf[0x400], *head;
	
	head=buf; len=(int*)&buf[4];
	if(mode&RDOnly)
	{
		ret|=_read(handle,buf,12);
		
		if((!memcmp(head  ,"RIFF",4)) && (!memcmp(head+8,"WAVE",4))) // найден WAV-заголовок
		{
			while(1)
			{
				_read(handle,buf,8);
				if(_eof(handle)) 
					break; // Exit
				if(!memcmp(head,fmt->name,4))
				{
					fmt->length=*len; // 0x12
					ret|=_read(handle,buf,*len);
					memcpy(fmt->Buf,buf,fmt->length);
				} 
				else
/*/
					if(!memcmp(head,fact->name,4))
					{
						fact->length=*len; // 0x4
						ret|=_read(handle,buf,*len);
						memcpy(fact->Buf,buf,fact->length);
					} 
					else
						if(!memcmp(head,freq->name,4))
						{
							freq->length=*len; // 0x4
							if(freq->length>256) freq->length=4;
							ret|=_read(handle,buf,freq->length);
							memcpy(freq->Buf,buf,freq->length);
						}
						else
							if(!memcmp(head,LIST->name,4))
							{
								LIST->length=*len; // 256
								ret|=_read(handle,buf,*len);
								memcpy(LIST->Buf,buf,LIST->length);
							} 
							else
*/
								if(!memcmp(head,data->name,4))
								{
									datalen=*len;
									data->length=*len;
									data->Buf=(char *)
										realloc(data->Buf, data->length);
									ret|=_read(handle,data->Buf,data->length);
								}
								else 
									_lseek(handle,*len,SEEK_CUR);
				if(ret==-1)     // Error
				{
					datalen=0; 
					break;
				} 
				if(_eof(handle)) // Exit
					break;          
			}
		}
		else // WAV-заголовок отсутствует
		{
			ptr=0; t1=data->length;
			while(1)
			{
				t2=_read(handle,data->Buf+ptr,t1);
				if(t2==-1) 
				{
					datalen=0; 
					break;
				}
				ptr+=t2; 
				if(t1!=t2) // eof
					break; 
				t1=0x20000; data->length+=t1;
				data->Buf=(char *)realloc(data->Buf, data->length);
			} 
			datalen=ptr; ptr=0;
			// запись нулевой частоты
			((fmt_*)fmt->Buf)->fmt_fsample=0;
//			*((float*)freq->Buf)=0.;
		}
	}
	return 0;
}
// закратие открытого файла
int	FileWAV::Close(void)
{
int	ret=0;
//unsigned attrib;
	if(mode)
	{
	  WriteToFile();
	  ret=_close(handle);

//	  SetFileAttributes(file,(GetFileAttributes(file)&~FILE_ATTRIBUTE_HIDDEN))	;
//	  _dos_getfileattr(file,&attrib);
//	  _dos_setfileattr(file,attrib&~_A_HIDDEN);
	}
	mode=0; handle=0;
	return ret;
}
// перемещение указателя чтения/записи
int	FileWAV::MovePtr(int offs,int m)
{
	if(!mode) return ptr;
	switch(m)
	{
	  case 0: ptr=offs;         break;
	  case 1: ptr=ptr+offs;     break;
	  case 2: ptr=datalen+offs; break;
	}
	if(ptr<0)       ptr=0;
	if(ptr>datalen) ptr=datalen;
	return ptr;
}
// запись последовательности отсчетов в WAV-структуру
int	FileWAV::Write(void *Buf,uint s)
{
	if(!(mode&WROnly)) return 0;
	if(ptr+s>data->length)
	{
	  data->length=ptr+s;
	  data->Buf=(char *) realloc(
		    data->Buf,
		    data->length);
	}
	memcpy(data->Buf+ptr,Buf,s);
	ptr+=s; datalen=ptr; FlagUpdate=TRUE;
	return s;
}
// чтение последовательности отсчетов из WAV-структуры
int	FileWAV::Read(void *Buf,uint s)
{
	if(!(mode&RDOnly)) return 0;
	if(ptr+s>datalen) s=datalen-ptr;
	if(s) memcpy(Buf,data->Buf+ptr,s);
	ptr+=s;
	return s;
}
// пересчет структуры fmt_
void    FileWAV::ReCalcFmt(void)
{       // (16/8=2) -> (1*8000*2=16000)
	fmt_ *format=(fmt_*)fmt->Buf;
	switch(format->fmt_format)
	{
	  case PCM:
		break;
	  case PCM8:
		format->fmt_bits=8;
		break;
	  case IEEE:
		  break;
	  case Alaw:
	  case Mlaw:
	  case IKM:
	  case IKM2:
		format->fmt_fsample=8000;
		format->fmt_bits=8;
		break;
	}
	format->fmt_bytes=
	  format->fmt_bits/8*
	  format->fmt_nChan;
	format->fmt_cps=
	  format->fmt_fsample*
	  format->fmt_bytes;

//	fact_ *fct=(fact_*)fact->Buf;
//	fct->fact_fsample=format->fmt_fsample;
}

// установка формата файла (PCM, Alaw, Mlaw)
void	FileWAV::SetFormat(ushort f)
{	((fmt_*)fmt->Buf)->fmt_format=f;  ReCalcFmt(); }
// чтение формата файла    (PCM, Alaw, Mlaw)
ushort	FileWAV::GetFormat(void)
{	return ((fmt_*)fmt->Buf)->fmt_format; }

// -----------------------------------------
//  установка режима регистрации mono/stereo
void FileWAV::SetFormatMono()
{
	((fmt_*)fmt->Buf)->fmt_nChan=1;
	ReCalcFmt();
}
void FileWAV::SetFormatStereo()
{
	((fmt_*)fmt->Buf)->fmt_nChan=2;
	ReCalcFmt();
}
// -----------------------------------------

// установка частоты дискретизации
void	FileWAV::SetFrequency(float f)
{
	((fmt_*)fmt->Buf)->fmt_fsample=(int)f;
//	*((float*)freq->Buf)=f;   ReCalcFmt();
}
// чтение частоты дискретизации
void	FileWAV::GetFrequency(int *f1,float *f2)
{
	*f1=((fmt_*)fmt->Buf)->fmt_fsample;
//	*f2=*((float*)freq->Buf);
}

// запись структуры fmt
void	FileWAV::SetFmt(fmt_ *param)
{	
	memcpy(fmt->Buf, param, fmt->length); 
	fmt_ *format=(fmt_*)fmt->Buf;
//	fact_ *fct=(fact_*)fact->Buf;
//	fct->fact_fsample=format->fmt_fsample;
}
// чтение структуры fmt
void	FileWAV::GetFmt(fmt_ *param)
{	memcpy(param, fmt->Buf, fmt->length); }
// запись структуры freq
void	FileWAV::SetFreq(float fr)
{
//	*(float*)freq->Buf=fr;
}
// чтение структуры freq
void	FileWAV::GetFreq(float *fr)
{
//	*fr=*(float*)freq->Buf;
}
// запись структуры LIST
void	FileWAV::SetLIST(char *t)
{
//	memcpy(LIST->Buf, t, LIST->length); 
}
// чтение структуры LIST
void	FileWAV::GetLIST(char *t)
{
//	memcpy(t, LIST->Buf, LIST->length); 
}

//------ Shot and Float Interface ----------------------------------------------

int	FileWAV::ReadShort(void *Buf,uint s)
{
int	i,cnt=0;
int ch, vcnt=0;
	switch(((fmt_*)fmt->Buf)->fmt_format)
	{
	  case PCM:
		   if(((fmt_*)fmt->Buf)->fmt_bits==8)
		   { // wav 8 bit
		     cnt=Read(Buf,s);
		     for(i=cnt-1;i>=0;i--)
				 ((short*)Buf)[i]=(short)(((uchar*)Buf)[i]-128);
		   }
		   else
		   {
		     cnt=Read(Buf,2*s)/2;
		   }
		   break;
	  case Alaw: // A-compand
			/*
		   cnt=Read(Buf,s);
		   for(i=cnt-1;i>=0;i--)
		   ((short*)Buf)[i]=Alaw2Short(((uchar*)Buf)[i]);
			*/
		    ch=((fmt_*)fmt->Buf)->fmt_nChan;
			vcnt=Read(Buf,s*ch); cnt=vcnt/ch;
			for(i=vcnt-1;i>=0;i--)
				((short*)Buf)[i]=Alaw2Short(((uchar*)Buf)[i]);
		   break;
	  case Mlaw:
		   cnt=Read(Buf,s);
		   for(i=cnt-1;i>=0;i--) // m-compand
		   ((short*)Buf)[i]=Mlaw2Short(((uchar*)Buf)[i]);
		   break;

	  case PCM8: // wav 8 bit
		   cnt=Read(Buf,s);
		   for(i=cnt-1;i>=0;i--)
		   ((short*)Buf)[i]=(short)(((uchar*)Buf)[i]-128);
		   break;
	  case IKM:
		   cnt=Read(Buf,s);
		   for(i=cnt-1;i>=0;i--) // rvt + A-compand
		   ((short*)Buf)[i]=Alaw2Short(rvt[((uchar*)Buf)[i]]);
		   break;
	  case IKM2:
		   cnt=Read(Buf,s);
		   for(i=cnt-1;i>=0;i--) // rvt + A-compand
		   ((short*)Buf)[i]=Mlaw2Short(rvt[((uchar*)Buf)[i]]);
		   break;
	}
	return cnt;
}
int	FileWAV::WriteChar(void *Buf,uint s)
{
	int	cnt=0;
	int ch, vcnt=0;
	uchar* pBuf=(uchar*)Buf;
	switch(((fmt_*)fmt->Buf)->fmt_format)
	{
	case PCM8:
	case Alw8:
	case Alaw:
		while(s)
		{
			sBlk=min(s,stBuf); s-=sBlk;
			ch=((fmt_*)fmt->Buf)->fmt_nChan;
			vcnt=sBlk*ch; 
// 			for(i=0;i<vcnt;i++)
// 				tBuf[i]=Short2Alaw(pBuf[i]);			
			vcnt=Write((void*)pBuf,vcnt); cnt=vcnt/ch;
			pBuf+=vcnt;
		}
		break;
	}
	return cnt;
}

int	FileWAV::WriteShort(void *Buf,uint s)
{
int	i,cnt=0;
int ch, vcnt=0;
short* pBuf=(short*)Buf;

	switch(((fmt_*)fmt->Buf)->fmt_format)
	{
	  case PCM:
		   if(((fmt_*)fmt->Buf)->fmt_bytes==1)
		   {
			 while(s)
			 {
			   sBlk=min(s,stBuf); s-=sBlk;

			   for(i=0;i<sBlk;i++) // a-compand
		       tBuf[i]=(uchar)(pBuf[i]+128);
		       cnt=Write(tBuf,sBlk);
			   pBuf+=sBlk;
			 }
		   }
		   if(((fmt_*)fmt->Buf)->fmt_bytes==2)
		   {
		     cnt=Write(Buf,2*s)/2;
		   }
		   break;
	  case Alaw: // A-compand
		   while(s)
		   {
		     sBlk=min(s,stBuf); s-=sBlk;

		     ch=((fmt_*)fmt->Buf)->fmt_nChan; vcnt=sBlk*ch; 
			 for(i=0;i<vcnt;i++)
			 tBuf[i]=Short2Alaw(pBuf[i]);
			 vcnt=Write(tBuf,vcnt); cnt=vcnt/ch;
		     pBuf+=sBlk;
		   }
		   break;
	  case Mlaw:
		   while(s)
		   {
		     sBlk=min(s,stBuf); s-=sBlk;

		     for(i=0;i<sBlk;i++) // m-compand
			 tBuf[i]=Short2Mlaw(pBuf[i]);
		     cnt=Write(tBuf,sBlk);
		     pBuf+=sBlk;
		   }
		   break;
	}
	return cnt;
}
int	FileWAV::ReadFloat(void *Buf,uint s)
{
int	i,cnt=0;
int ch, vcnt=0;
	switch(((fmt_*)fmt->Buf)->fmt_format)
	{
	  case PCM:
		  switch (((fmt_*)fmt->Buf)->fmt_bits)
		  {
		  case 8:
			  cnt = Read(Buf, s);
			  for (i = cnt - 1; i >= 0; i--)
				  ((float*)Buf)[i] = (float)(((uchar*)Buf)[i] - 128);
			  break;
		  case 16:
			  cnt = Read(Buf, 2 * s) / 2;
			  for (i = cnt - 1; i >= 0; i--) // short
				  ((float*)Buf)[i] = (float)((short*)Buf)[i];
			  break;
		  case 24:
			  cnt = Read(Buf, 3 * s) / 3;
			  for (i = cnt - 1; i >= 0; i--) // short
			  {	// 24 -> 32
				  ((uchar*)Buf)[i * 4] = ((uchar*)Buf)[i * 3];
				  ((uchar*)Buf)[i * 4 + 1] = ((uchar*)Buf)[i * 3 + 1];
				  ((uchar*)Buf)[i * 4 + 2] = ((uchar*)Buf)[i * 3 + 2];
				  if ((((uchar*)Buf)[i * 4 + 2]>>7)!=0)
					  ((uchar*)Buf)[i * 4 + 3] = 0xff;
				  else
					  ((uchar*)Buf)[i * 4 + 3] = 0;
			  }
			  for (i = cnt - 1; i >= 0; i--) // short
				  ((float*)Buf)[i] = (float)((int*)Buf)[i];
			  break;
		  case 32:
			  cnt = Read(Buf, 4 * s) / 4;
			  for (i = cnt - 1; i >= 0; i--) // short
				  ((float*)Buf)[i] = (float)((int*)Buf)[i];
			  break;
		  }
		  break;
	  case IEEE:
		  switch (((fmt_*)fmt->Buf)->fmt_bits)
		  {
		  case 32:
//			  cnt = Read(Buf, 4 * s) / 4;
//			  for (i = cnt - 1; i >= 0; i--) // short
//				  ((float*)Buf)[i] = (float)((int*)Buf)[i];
			  break;
		  case 64:
			  cnt = Read(Buf, 8 * s) / 8;
			  for (i = 0; i < cnt;  i++) // short
				  ((float*)Buf)[i] = (float)((double*)Buf)[i];
			  break;
		  }
		  break;
	  case Alaw: // A-compand
		   /*
		   cnt=Read(Buf,s);
		   for(i=cnt-1;i>=0;i--)
		   ((float*)Buf)[i]=(float)Alaw2Short(((uchar*)Buf)[i]);
		   */
		    ch=((fmt_*)fmt->Buf)->fmt_nChan;
			vcnt=Read(Buf,s*ch); cnt=vcnt/ch;
			for(i=vcnt-1;i>=0;i--)
				((float*)Buf)[i]=(float)Alaw2Short(((uchar*)Buf)[i]);
		   break;
	  case Mlaw:
		   cnt=Read(Buf,s);
		   for(i=cnt-1;i>=0;i--) // m-compand
		   ((float*)Buf)[i]=(float)Mlaw2Short(((uchar*)Buf)[i]);
		   break;

	  case PCM8: // wav 8 bit
		   cnt=Read(Buf,s);
		   for(i=cnt-1;i>=0;i--)
		   ((float*)Buf)[i]=(float)(((uchar*)Buf)[i]-128);
		   break;
	  case IKM:
		   cnt=Read(Buf,s);
		   for(i=cnt-1;i>=0;i--) // rvt + A-compand
		   ((float*)Buf)[i]=(float)Alaw2Short(rvt[((uchar*)Buf)[i]]);
		   break;
	  case IKM2:
		   cnt=Read(Buf,s);
		   for(i=cnt-1;i>=0;i--) // rvt + A-compand
		   ((float*)Buf)[i]=(float)Mlaw2Short(rvt[((uchar*)Buf)[i]]);
		   break;
	}
	return cnt;
}
int	FileWAV::WriteFloat(void *Buf,uint s)
{
int	i,cnt=0;
int ch, vcnt=0;
float* pBuf=(float*)Buf;
	if (handle!=-1)
	switch(((fmt_*)fmt->Buf)->fmt_format)
	{
	  case PCM:
		   if(((fmt_*)fmt->Buf)->fmt_nChan==2)
			   s*=2;
		   if(((fmt_*)fmt->Buf)->fmt_bytes==1)
		   {
			 while(s)
			 {
			   sBlk=min(s,stBuf); s-=sBlk;

			   for(i=0;i<sBlk;i++)
		       tBuf[i]=(uchar)(pBuf[i]+128);
		       cnt=Write(tBuf,sBlk);
			   pBuf+=sBlk;
			 }
		   }
		   if(
			   (((fmt_*)fmt->Buf)->fmt_bytes==2) ||
			   (((fmt_*)fmt->Buf)->fmt_bytes==4)
				   )
		   {
			 while(s)
			 {
			   sBlk=min(s,stBuf); s-=sBlk;

			   for(i=0;i<sBlk;i++) // int
		       ((short*)tBuf)[i]=(short)pBuf[i];
		       cnt=Write(tBuf,2*sBlk)/2;
			   pBuf+=sBlk;
			 }
		   }
		   break;
	  case IEEE:
		  if (((fmt_*)fmt->Buf)->fmt_nChan == 2)
			  s *= 2;
		  if (((fmt_*)fmt->Buf)->fmt_bytes == 4)
		  {
			  while (s)
			  {
				  sBlk = min(s, stBuf);
				  s -= sBlk;
				  for (i = 0; i < sBlk; i++) // int
					  ((float*)tBuf)[i] = (float)pBuf[i];
				  cnt = Write(tBuf, 4 * sBlk) / 4;
				  pBuf += sBlk;
			  }
		  }
		  else
		  if (((fmt_*)fmt->Buf)->fmt_bytes == 8)
		  {
			  while (s)
			  {
				  sBlk = min(s, stBuf);
				  s -= sBlk;
				  for (i = 0; i < sBlk; i++) // int
					  ((float*)tBuf)[i] = (float)pBuf[i];
				  cnt = Write(tBuf, 4 * sBlk) / 4;
				  pBuf += sBlk;
			  }
		  }

		  break;
	  case Alaw: // A-compand
		   while(s)
		   {
		     sBlk=min(s,stBuf); s-=sBlk;

		     ch=((fmt_*)fmt->Buf)->fmt_nChan; vcnt=sBlk*ch; 
			 for(i=0;i<vcnt;i++)
			 tBuf[i]=Float2Alaw(pBuf[i]);
			 vcnt=Write(tBuf,vcnt); cnt=vcnt/ch;
			 pBuf+=sBlk;
		   }
		   break;
	  case Mlaw:
		   while(s)
		   {
		     sBlk=min(s,stBuf); s-=sBlk;

		     for(i=0;i<sBlk;i++) // m-compand
		     tBuf[i]=Float2Mlaw(pBuf[i]);
		     cnt=Write(tBuf,sBlk);
			 pBuf+=sizeof(float)*sBlk;
		   }
		   break;
	}
	return cnt;
}
int	FileWAV::MovePtrUsr(int offs,int m)
{
	int ch=((fmt_*)fmt->Buf)->fmt_nChan;
	switch(((fmt_*)fmt->Buf)->fmt_format)
	{
	  case Alaw:
	  case Mlaw:
	  case PCM8:
	  case IKM:
	  case IKM2:
		      return MovePtr(offs*ch,m)/ch;
	  case PCM:   if(((fmt_*)fmt->Buf)->fmt_bytes==1)
			return MovePtr(offs,m); else
		      if(((fmt_*)fmt->Buf)->fmt_bytes==2)
			return MovePtr(2*offs,m)/2;	
	}
	return 0;
}
//------------------------------------------------------------------------------
FileWAV_VeryFast::FileWAV_VeryFast(uint dBuf) : FileWAV(dBuf)
{
	A1=A0=buflen=0;
}
FileWAV_VeryFast::~FileWAV_VeryFast()
{

//	FileWAV::~FileWAV();
}

// создание нового файла
int	FileWAV_VeryFast::Create(char *f)
{
	strcpy_s(file,f); //_fmode=O_BINARY;
	if(handle<1)  _sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, _SH_DENYNO, S_IREAD | S_IWRITE);
	mode=WROnly; A1=A0=0;
	buflen=datalen=0;
	FlagUpdate=FALSE;
	if(handle!=-1)
	{
	  RIFF->length=4+ 8+8+8+8+
		       fmt ->length+
//		       fact->length+
//		       freq->length+
//		       LIST->length+
		       data->length;

	  _write(handle, RIFF->name, 8);
	  _write(handle, RIFF->Buf, 4);

	  _write(handle, fmt->name, 8);
//	  fmt->fileptr=lseek(handle,0,SEEK_CUR);
	  fmt->fileptr=_lseeki64(handle,0,SEEK_CUR);
	  _write(handle, fmt->Buf, fmt->length);
/*
	  _write(handle, fact->name, 8);
// 	  fact->fileptr=lseek(handle,0,SEEK_CUR);
	  fact->fileptr=_lseeki64(handle,0,SEEK_CUR);
	  _write(handle, fact->Buf, fact->length);

	  freq->length+=0x108;
	  _write(handle, freq->name, 8);
	  freq->length-=0x108;
//	  freq->fileptr=lseek(handle,0,SEEK_CUR);
	  freq->fileptr=_lseeki64(handle,0,SEEK_CUR);
	  _write(handle, freq->Buf, freq->length);

	  _write(handle, LIST->name, 8);
//	  LIST->fileptr=lseek(handle,0,SEEK_CUR);
	  LIST->fileptr=_lseeki64(handle,0,SEEK_CUR);
	  _write(handle, LIST->Buf, LIST->length);
*/
	  _write(handle, data->name, 8);
//	  data->fileptr=lseek(handle,0,SEEK_CUR);
	  data->fileptr=_lseeki64(handle,0,SEEK_CUR);
	}
	return handle;
}

//
int	FileWAV_VeryFast::Create3(char* cat, char* ext, INT64 ID, int Number)
{
	SYSTEMTIME	TTime;
	FileTime		OperTime;
	int	num = 0; char t_buf[80] = "", tn[12];
	strcpy(t_buf, cat);
	CalcPath(t_buf);
	MakeDir(t_buf);
	GetSystemTime(&TTime);
	sprintf(t_buf,"%s%04d%02d%02d_%I64x_%d%s", t_buf, TTime.wYear, TTime.wMonth, TTime.wDay, ID, Number, ext);
	_fmode = O_BINARY;
	handle = _open(t_buf, O_CREAT | O_EXCL | O_BINARY | O_RDWR);
	return Create(t_buf);
}

int	FileWAV_VeryFast::Create4(char* cat, char* ext, int Number)
{
	int	num = 0; char t_buf[80] = "", tn[12];
	strcpy(t_buf, cat);
	CalcPath(t_buf);
	MakeDir(t_buf);
	strcat(t_buf, CalcNameData(tn, Number));
	strcat(t_buf, ext); _fmode = O_BINARY;
	handle = _open(t_buf, O_CREAT | O_EXCL | O_BINARY | O_RDWR);
	return Create(t_buf);
}

// открытие существующего файла
int	FileWAV_VeryFast::Open(char *f, int m)
{
	strcpy_s(file,f); mode=m+1;
//	_fmode=O_BINARY;
	switch(mode)
	{
	case RDOnly: _sopen_s(&handle, file, O_RDONLY | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE); break;
	case WROnly: _sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE); break;
	case RDWR: _sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);   break;
	}
	if (handle==-1)
	{
		mode=0; return handle;
	}
	DataLen64=_lseeki64(handle,0,SEEK_END);
	ReadFromFileFirst(); // чтение из файла (buflen,datalen)
	A1=A0=0; FlagUpdate=FALSE;
	return handle;
}
// -------------------------------------------------------------------
uint FileWAV_VeryFast::pread64(int h,void *b,uint c/*,INT64 off*/) 
{
//	_lseeki64(h,off,0);
	return _read(h,b,c);
}

/*
uint FileWAV::pwrite64(int h,void *b,uint c,INT64 off) 
{
	_lseeki64(h,off,0);
	return _write(h,b,c);
}
*/

INT64 FileWAV_VeryFast::eof64(int h) 
{
	INT64 rc;

	rc=_telli64(handle);
	if((rc==-1) || (rc>=DataLen64))
		rc=-1;
	else
		rc=0;

	return rc;
}

// -------------------------------------------------------------------
// первое чтение из открытого файла
int	FileWAV_VeryFast::ReadFromFileFirst(void)
{
	int	ret=0,*len;
	char buf[0x400], *head;

	head=buf; len=(int*)&buf[4];
	if(mode&RDOnly)
	{
//		ret|=read(handle,buf,12);
		fmt->fileptr=_lseeki64(handle,0,SEEK_SET);
		ret|=pread64(handle,buf,12);

		if((!memcmp(head ,"RIFF",4)) && (!memcmp(head+8,"WAVE",4))) // найден WAV-заголовок
		{
			while(1)
			{
//				read(handle,buf,8);
				pread64(handle,buf,8);
//				if(eof(handle)) 
				if(eof64(handle))
					break; // Exit

				if(!memcmp(head,fmt->name,4))
				{
					fmt->length=*len; // 0x12
//					fmt->fileptr=lseek(handle,0,SEEK_CUR);
//					ret|=read(handle,buf,*len);
					fmt->fileptr=_lseeki64(handle,0,SEEK_CUR);
					ret|=pread64(handle,buf,*len);
					memcpy(fmt->Buf,buf,fmt->length);
				} 
				else
/*
				  if(!memcmp(head,fact->name,4))
				  {
					  fact->length=*len; // 0x4
//					  fact->fileptr=lseek(handle,0,SEEK_CUR);
//					  ret|=read(handle,buf,*len);
					  fact->fileptr=_lseeki64(handle,0,SEEK_CUR);
					  ret|=pread64(handle,buf,*len);
					  memcpy(fact->Buf,buf,fact->length);
				  } 
				  else
					  if(!memcmp(head,freq->name,4))
					  {
						  freq->length=*len; // 0x4
						  if(freq->length>256)
							  freq->length=4;
//						  freq->fileptr=lseek(handle,0,SEEK_CUR);
//						  ret|=read(handle,buf,freq->length);
						  freq->fileptr=_lseeki64(handle,0,SEEK_CUR);
						  ret|=pread64(handle,buf,freq->length);
						  memcpy(freq->Buf,buf,freq->length);
					  }
					  else
						  if(!memcmp(head,LIST->name,4))
						  {
							  LIST->length=*len; // 256
//							  LIST->fileptr=lseek(handle,0,SEEK_CUR);
//							  ret|=read(handle,buf,*len);
							  LIST->fileptr=_lseeki64(handle,0,SEEK_CUR);
							  ret|=pread64(handle,buf,*len);
							  memcpy(LIST->Buf,buf,LIST->length);
						  }
						  else
*/
							  if(!memcmp(head,data->name,4))
							  {
								  datalen=*len;
//								  data->fileptr=lseek(handle,0,SEEK_CUR);
//								  buflen=read(handle,data->Buf,data->length);
//								  lseek(handle,(datalen-data->length),SEEK_CUR);
								  data->fileptr=_lseeki64(handle,0,SEEK_CUR);
								  buflen=pread64(handle,data->Buf,data->length);
								  _lseeki64(handle,(datalen-data->length),SEEK_CUR);
							  }
							  else 
//								  lseek(handle,*len,SEEK_CUR);
								  _lseeki64(handle,*len,SEEK_CUR);
							  if(ret==-1) // Error
							  {
								  datalen=0; 
								  break;
							  } 
//							  if(eof(handle)) // Exit
							  if(eof64(handle)) // Exit
								  break;          
			}
		}
		else // WAV-заголовок отсутствует	  
		{
			mode&=~WROnly; // отрубить запись
//			data->fileptr=lseek(handle,0,SEEK_CUR);
//			buflen=read(handle,data->Buf,data->length);
//			datalen=lseek(handle,0,SEEK_END)-data->fileptr;
			data->fileptr=_lseeki64(handle,0,SEEK_CUR);
			buflen=pread64(handle,data->Buf,data->length);
			datalen=_lseeki64(handle,0,SEEK_END)-data->fileptr;
			((fmt_*)fmt->Buf)->fmt_fsample=0; //ReCalcFmt();
//			*((float*)freq->Buf)=0.;
		}
	}
	return 0;
}
// сохранение буфера в файле
int	FileWAV_VeryFast::WriteToFile(void)
{
	int	cnt=0;
	if((mode&WROnly)&&(FlagUpdate))
	{
		FlagUpdate=FALSE;
//		lseek(handle,data->fileptr+A1,SEEK_SET);
		_lseeki64(handle,data->fileptr+A1,SEEK_SET);
		cnt=_write(handle,data->Buf,A0); buflen=0;
	}
	return cnt;
}
// чтение в буфер
int	FileWAV_VeryFast::ReadFromFile(void)
{
	if(mode&RDOnly)
	{
//		lseek(handle,data->fileptr+A1,SEEK_SET);
		_lseeki64(handle,data->fileptr+A1,SEEK_SET);
		buflen=_read(handle,data->Buf,data->length);
	}
	return buflen;
}

// закратие открытого файла
int	FileWAV_VeryFast::Close(void)
{
	int	ret=0;
	if(mode)
	{
//		ret=_close(handle);
//		handle = _open(file, O_CREAT | O_EXCL | O_BINARY | O_RDWR);
//		_sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);
		if(handle!=-1)
		{
			WriteToFile();
			//close(handle);
		
		UpdateLenRIFF();
//		SetFileAttributes(file,(GetFileAttributes(file)&~FILE_ATTRIBUTE_HIDDEN))	;
		_close(handle);
		}
	}
	mode=0; handle=0;
	return ret;
}
// перемещение указателя чтения/записи
int	FileWAV_VeryFast::MovePtr(int offs,int m)
{
int	ptr;
	ptr=A1+A0; if(!mode) return ptr;
	switch(m)
	{
	  case 0: ptr=offs;         break;
	  case 1: ptr=ptr+offs;     break;
	  case 2: ptr=datalen+offs; break;
	}
	if(ptr<0)       ptr=0;
	if(ptr>datalen) ptr=datalen;
	// ReRead
	if((A1<=ptr)&&(ptr<A1+buflen))
	{ A0=ptr-A1; }
	else
	{ A1=ptr; A0=0; ReadFromFile();	}
	return ptr;
}
// байтовая запись
int	FileWAV_VeryFast::Write(void *Buf,uint s)
{       // поле data должно быть последним !!!
int	t1,t2, cnt=0;;
	if(mode&WROnly)
	while((s)/*&&(A1+A0<datalen)*/)
	{
	  FlagUpdate=TRUE;
	  t1=data->length-A0; t2=(s>t1)? t1 : s;
	  memcpy(data->Buf+A0,(char*)Buf+cnt,t2);
	  A0+=t2; cnt+=t2; s-=t2;
	  if(A0>buflen)     buflen=A0;     // coor buflen
	  if(A1+A0>datalen) datalen=A1+A0; // corr datalen
	  if((A0==data->length)/*&&(A1+A0<datalen)*/)
	  { WriteToFile(); A1+=A0; A0=0; }
	}
	return cnt;
}
// байтовое чтение
int	FileWAV_VeryFast::Read(void *Buf,uint s)
{
int	t1,t2, cnt=0;;
	if(mode&RDOnly)
	while((s)&&(A1+A0<datalen))
	{
	  t1=buflen-A0; t2=(s>t1)? t1 : s;
	  memcpy((char*)Buf+cnt,data->Buf+A0,t2);
	  A0+=t2; cnt+=t2; s-=t2;
	  if((A0==buflen)&&(A1+A0<datalen))
	  { A1+=A0; A0=0; ReadFromFile(); }
	}
	return cnt;
}

// установка формата файла (PCM, Alaw, Mlaw)
void	FileWAV_VeryFast::SetFormat(ushort f)
{
	((fmt_*)fmt->Buf)->fmt_format=f;
	ReCalcFmt(); if(f<129) UpdateTag(fmt);
}
// чтение формата файла    (PCM, Alaw, Mlaw)
ushort	FileWAV_VeryFast::GetFormat(void)
{	return ((fmt_*)fmt->Buf)->fmt_format; }
// установка частоты дискретизации
void	FileWAV_VeryFast::SetFrequency(float f)
{
	((fmt_*)fmt->Buf)->fmt_fsample=(int)f;
//	*((float*)freq->Buf)=f;   ReCalcFmt();
	UpdateTag(fmt);
//	UpdateTag(freq);
}
// чтение частоты дискретизации
void	FileWAV_VeryFast::GetFrequency(int *f1,float *f2)
{
	*f1=((fmt_*)fmt->Buf)->fmt_fsample;
//	*f2=*((float*)freq->Buf);
}
// запись структуры fmt
void	FileWAV_VeryFast::SetFmt(fmt_ *param)
{	
	memcpy(fmt->Buf, param, fmt->length); 
	fmt_ *format=(fmt_*)fmt->Buf;
//	fact_ *fct=(fact_*)fact->Buf;
//	fct->fact_fsample=format->fmt_fsample;
	UpdateTag(fmt);
//	UpdateTag(fact);
}
// чтение структуры fmt
void	FileWAV_VeryFast::GetFmt(fmt_ *param)
{ 	memcpy(param, fmt->Buf, fmt->length); }
// запись структуры freq
void	FileWAV_VeryFast::SetFreq(float fr)
{ 
//	*(float *)freq->Buf=fr; UpdateTag(freq);
}
// чтение структуры freq
void	FileWAV_VeryFast::GetFreq(float *fr)
{
//	*fr=*(float *)freq->Buf;
}
// запись структуры LIST
void	FileWAV_VeryFast::SetLIST(char *t)
{
//	memcpy(LIST->Buf, t, LIST->length); UpdateTag(LIST);
}
// чтение структуры LIST
void	FileWAV_VeryFast::GetLIST(char *t)
{
//	memcpy(t, LIST->Buf, LIST->length);
}

// обновление поля WAV-файла
void	FileWAV_VeryFast::UpdateTag(Tag *t)
{
	if(mode&WROnly)
	{
//	  lseek(handle,t->fileptr,SEEK_SET);
	  _lseeki64(handle,t->fileptr,SEEK_SET);
	  _write(handle,t->Buf,t->length);
	}
}
void	FileWAV_VeryFast::UpdateLenRIFF(void)
{
int	temp;
	if(mode&WROnly)
	{
//	  lseek(handle,data->fileptr-8,SEEK_SET);
	  _lseeki64(handle,data->fileptr-8,SEEK_SET);
	  temp=data->length;
	  data->length=datalen;
	  _write(handle, data->name, 8);
	  data->length=temp;

//	  lseek(handle,0L,SEEK_SET);
	  _lseeki64(handle,0L,SEEK_SET);
	  RIFF->length=4+ 8+8+8+8+
		       fmt ->length+
//		       fact->length+
//		       freq->length+
//		       LIST->length+
		       datalen;
	  _write(handle, RIFF->name, 8);
	}
}
//------------------------------------------------------------------------------
FileWAV_VeryFast2::FileWAV_VeryFast2(uint dBuf) : FileWAV_VeryFast(dBuf) { }
FileWAV_VeryFast2::~FileWAV_VeryFast2() {/* FileWAV_VeryFast::~FileWAV_VeryFast();*/ }

// создание нового файла
int	FileWAV_VeryFast2::Create(char *f)
{
	strcpy_s(file,f);// _fmode=O_BINARY;
	if(handle<1)  _sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);
	mode=WROnly; A1=A0=0;
	buflen=0; datalen=0;
	FlagUpdate=FALSE;
	if(handle!=-1)
	{
	  RIFF->length=4+ 8+8+8+8+
		       fmt ->length+
//		       freq->length+
//		       fact->length+
//		       LIST->length+
		       data->length;

	  _write(handle, RIFF->name, 8);
	  _write(handle, RIFF->Buf, 4);

	  _write(handle, fmt->name, 8);
	  fmt->fileptr=_lseek(handle,0,SEEK_CUR);
	  _write(handle, fmt->Buf, fmt->length);
/*
	  _write(handle, fact->name, 8);
	  fact->fileptr=_lseek(handle,0,SEEK_CUR);
	  _write(handle, fact->Buf, fact->length);

	  freq->length+=0x108;
	  _write(handle, freq->name, 8);
	  freq->length-=0x108;
	  freq->fileptr=_lseek(handle,0,SEEK_CUR);
	  _write(handle, freq->Buf, freq->length);

	  _write(handle, LIST->name, 8);
	  LIST->fileptr=_lseek(handle,0,SEEK_CUR);
	  _write(handle, LIST->Buf, LIST->length);
*/
	  _write(handle, data->name, 8);
	  data->fileptr=_lseek(handle,0,SEEK_CUR);

	  _close(handle);
	}
	return handle;
}
// открытие существующего файла
int	FileWAV_VeryFast2::Open(char *f, int m)
{
	strcpy_s(file,f); mode=m+1;
//	_fmode=O_BINARY;
	switch(mode)
	{
	  case RDOnly: _sopen_s(&handle,file, O_RDONLY | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE); break;
	  case WROnly: _sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE); break;
	  case RDWR  : _sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);   break;
	}
	if (handle==-1) {mode=0; return handle;}
	ReadFromFileFirst(); // чтение из файла (buflen,datalen)
	A1=A0=0; FlagUpdate=FALSE;
	_close(handle);
	return handle;
}
// сохранение буфера в файле
int	FileWAV_VeryFast2::WriteToFile(void)
{
int	cnt=0;
	if((mode&WROnly)&&(FlagUpdate))
	{
	  FlagUpdate=FALSE;
	  _sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);
	  _lseek(handle,data->fileptr+A1,SEEK_SET);
	  cnt=_write(handle,data->Buf,A0); buflen=0;
	  UpdateLenRIFF();
	  _close(handle);
	}
	return cnt;
}
// чтение в буфер
int	FileWAV_VeryFast2::ReadFromFile(void)
{
	if(mode&RDOnly)
	{
	  _sopen_s(&handle,file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);
	  _lseek(handle,data->fileptr+A1,SEEK_SET);
	  buflen=_read(handle,data->Buf,data->length);
	  _close(handle);
	}
	return buflen;
}
// закратие открытого файла
int	FileWAV_VeryFast2::Close(void)
{
int	ret=0;
//unsigned attrib;
	if(mode)
	{
	  WriteToFile();
//	  SetFileAttributes(file,(GetFileAttributes(file)&~FILE_ATTRIBUTE_HIDDEN))	;
//	  _dos_getfileattr(file,&attrib);
//	  _dos_setfileattr(file,attrib&~_A_HIDDEN);
	}
	mode=0; handle=0;
	return ret;
}
// обновление поля WAV-файла
void	FileWAV_VeryFast2::UpdateTag(Tag *t)
{
	if(mode&WROnly)
	{
		_sopen_s(&handle, file,O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);
	  _lseek(handle,t->fileptr,SEEK_SET);
	  _write(handle,t->Buf,t->length);
	  _close(handle);
	}
}
// -----------------------------------------------------------------------------

UCHAR RIFF_Header[]={
0x52,0x49,0x46,0x46,0x46,0x01,0x00,0x00,0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20, //  0
0x12,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x9A,0x5B,0x06,0x00,0x34,0xB7,0x0C,0x00, //  1
0x02,0x00,0x10,0x00,0x00,0x00,0x66,0x61,0x63,0x74,0x04,0x00,0x00,0x00,0x40,0x1F, //  2
0x00,0x00,0x66,0x72,0x65,0x71,0x04,0x00,0x00,0x00,0x55,0x73,0xCB,0x48,0x4C,0x49, //  3
0x53,0x54,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  4
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  5
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  6
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  7
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  8
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  9
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  a
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  b
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  c
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  d
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  e
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //  f
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 10
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 11
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 12
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 13
0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x61,0x74,0x61,0x00,0x00,0x00,0x00 };         // 14

	FileWAV_VeryFastNewYearVersion::FileWAV_VeryFastNewYearVersion(uint SizeBuf,int iGain)
{
	Gain=iGain;
	sBuf=SizeBuf;
	Buf=(PUCHAR)malloc(4*sBuf);
	*file=0; handle=0; counter=0; Format=1; NumberCh=1;
}

	FileWAV_VeryFastNewYearVersion::~FileWAV_VeryFastNewYearVersion()
{
	free(Buf);
}

	int	FileWAV_VeryFastNewYearVersion::Create(char *f)
{
	strcpy_s(file,f);// _fmode=O_BINARY;
	if(handle<1) 	  _sopen_s(&handle, file, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);
	return handle;
}

int	FileWAV_VeryFastNewYearVersion::Create2(char *cat,char *ext)
{
int	num=0; char t_buf[80]="",tn[9];
	while (1)
	{
	  strcpy_s(t_buf,cat);
	  CalcPath(t_buf);
//  	  MakeDir(t_buf);
//	  strcat_s(t_buf,CalcNameData(tn,num));
	  strcat_s(t_buf,ext);// _fmode=O_BINARY;
	  _sopen_s(&handle,t_buf, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);
//	  if (handle!=-1) SetFileAttributes(t_buf,FILE_ATTRIBUTE_HIDDEN);

//	  handle=creatnew(t_buf, FA_HIDDEN);
	  if(handle!=-1) break; else num++;
	  
	  if(num==1000) break; // gluk
	}
	return Create(t_buf);
}

int	FileWAV_VeryFastNewYearVersion::Create2Local(char *cat,char *ext)
{
int	num=0; char t_buf[80]="",tn[9];
	while (1)
	{
	  strcpy_s(t_buf,cat);
	  CalcPath(t_buf);
//  	  MakeDir(t_buf);
//	  strcpy_s(Namefile,CalcNameDataLocal(tn,num));
	  strcat_s(t_buf,Namefile);
	  strcat_s(t_buf,ext);// _fmode=O_BINARY;
	  _sopen_s(&handle,t_buf, O_CREAT | O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE);
//	  if (handle!=-1) SetFileAttributes(t_buf,FILE_ATTRIBUTE_HIDDEN);

//	  handle=creatnew(t_buf, FA_HIDDEN);
	  if(handle!=-1) break; else num++;
	  
	  if(num==1000) break; // gluk
	}
	return Create(t_buf);
}

void	FileWAV_VeryFastNewYearVersion::SetFormat(ushort fmt)
{
	//Format=fmt; 
	Format=1;//???
}


void	FileWAV_VeryFastNewYearVersion::SetFormatStereo()
{
	NumberCh=2;
}

void	FileWAV_VeryFastNewYearVersion::SetFrequency(float fd)
{
	memcpy(LocalHeader,RIFF_Header,sizeof(RIFF_Header));
	
	counter=0;

	*((short*)(LocalHeader+0x14))=Format;
	*((short*)(LocalHeader+0x16))=NumberCh;
	*((int*)(LocalHeader+0x18))=(int)fd;
	*((int*)(LocalHeader+0x1c))=2*NumberCh*((int)fd);
	*((float*)(LocalHeader+0x3a))=fd;
	*((short*)(LocalHeader+0x20))=2*NumberCh;
	_write(handle,LocalHeader,sizeof(RIFF_Header));
}

int	FileWAV_VeryFastNewYearVersion::WriteFloat(void *b,uint s)
{
	int i,count;
	float* BufIn=(float*)b;
	short* BufOut;
		
	if(handle)
	{
		while(s)
		{
			count=s; if(count>sBuf) count=sBuf;
			BufOut=(short*)Buf;
			for(i=0;i<count;i++)
			{	
				*BufOut++=(short)((*BufIn)*Gain);
				BufIn++;
			}
			_write(handle,Buf,2*count);
			counter+=2*count;
			s-=count;
		}
	}
	return 0;
}

int	FileWAV_VeryFastNewYearVersion::Close()
{
	if(handle)
	{
		*((int*)(LocalHeader+0x4))+=counter;
		*((int*)(LocalHeader+0x14a))+=counter;
		_lseek(handle,0L,SEEK_SET);
		_write(handle,LocalHeader,sizeof(RIFF_Header));

		_close(handle); handle=0;

//		SetFileAttributes(file,(GetFileAttributes(file)&~FILE_ATTRIBUTE_HIDDEN));
	}
	return 0;
}
