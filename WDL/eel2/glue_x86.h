#ifndef _NSEEL_GLUE_X86_H_
#define _NSEEL_GLUE_X86_H_

#define GLUE_MAX_FPSTACK_SIZE 8
#define GLUE_JMP_TYPE int
#define GLUE_JMP_OFFSET 0 // offset from end of instruction that is the "source" of the jump
#define GLUE_JMP_OFFSET_MASK 0xffffffff

static const unsigned char GLUE_JMP_NC[] = { 0xE9, 0,0,0,0, }; // jmp<offset>
static const unsigned char GLUE_JMP_IF_P1_Z[] = {0x85, 0xC0, 0x0F, 0x84, 0,0,0,0 }; // test eax, eax, jz
static const unsigned char GLUE_JMP_IF_P1_NZ[] = {0x85, 0xC0, 0x0F, 0x85, 0,0,0,0 }; // test eax, eax, jnz

#define GLUE_FUNC_ENTER_SIZE 0
#define GLUE_FUNC_LEAVE_SIZE 0
const static unsigned int GLUE_FUNC_ENTER[1];
const static unsigned int GLUE_FUNC_LEAVE[1];

  // x86
  // stack is 16 byte aligned
  // when pushing values to stack, alignment pushed first, then value (value is at the lower address)
  // when pushing pointers to stack, alignment pushed first, then pointer (pointer is at the lower address)

  static const unsigned char GLUE_PUSH_P1PTR_AS_VALUE[] = 
  { 
    0x83, 0xEC, 8, /* sub esp, 8 */   
    0xff, 0x70, 0x4, /* push dword [eax+4] */ 
    0xff, 0x30, /* push dword [eax] */
  };

  static int GLUE_POP_VALUE_TO_ADDR(unsigned char *buf, void *destptr)
  {    
    if (buf)
    {
      *buf++ = 0xB8; *(void **) buf = destptr; buf+=4; // mov eax, directvalue
           
      *buf++ = 0x8f; *buf++ = 0x00; // pop dword [eax]
      *buf++ = 0x8f; *buf++ = 0x40; *buf++ = 4; // pop dword [eax+4]
      
      *buf++ = 0x59; // pop ecx (alignment)
      *buf++ = 0x59; // pop ecx (alignment)
    }
    
    return 12;
  }

  static int GLUE_COPY_VALUE_AT_P1_TO_PTR(unsigned char *buf, void *destptr)
  {    
    if (buf)
    {
      *buf++ = 0x8B; *buf++ = 0x38; // mov edi, [eax]
      *buf++ = 0x8B; *buf++ = 0x48; *buf++ = 0x04; // mov ecx, [eax+4]
      
      
      *buf++ = 0xB8; *(void **) buf = destptr; buf+=4; // mov eax, directvalue
      *buf++ = 0x89; *buf++ = 0x38; // mov [eax], edi
      *buf++ = 0x89; *buf++ = 0x48; *buf++ = 0x04; // mov [eax+4], ecx
    }
    
    return 2 + 3 + 5 + 2 + 3;
  }

  static int GLUE_POP_FPSTACK_TO_PTR(unsigned char *buf, void *destptr)
  {
    if (buf)
    {
      *buf++ = 0xB8; *(void **) buf = destptr; buf+=4; // mov eax, directvalue
      *buf++ = 0xDD; *buf++ = 0x18;  // fstp qword [eax]
    }
    return 1+4+2;
  }


  #define GLUE_MOV_PX_DIRECTVALUE_SIZE 5
  #define GLUE_MOV_PX_DIRECTVALUE_TOSTACK_SIZE 6 // length when wv == -1

  static void GLUE_MOV_PX_DIRECTVALUE_GEN(void *b, INT_PTR v, int wv) 
  {   
    if (wv==-1)
    {
      const static unsigned char t[2] = {0xDD, 0x05};
      memcpy(b,t,2);
      b= ((unsigned char *)b)+2;
    }
    else
    {
      const static unsigned char tab[3] = {
        0xB8 /* mov eax, dv*/, 
        0xBF /* mov edi, dv */ , 
        0xB9 /* mov ecx, dv */ 
      };
      *((unsigned char *)b) = tab[wv];  // mov eax, dv
      b= ((unsigned char *)b)+1;
    }
    *(INT_PTR *)b = v; 
  }
  const static unsigned char  GLUE_PUSH_P1[4]={0x83, 0xEC, 12,   0x50}; // sub esp, 12, push eax
  
  #define GLUE_POP_PX_SIZE 4
  static void GLUE_POP_PX(void *b, int wv)
  {
    static const unsigned char tab[3][GLUE_POP_PX_SIZE]=
    {
      {0x58,/*pop eax*/  0x83, 0xC4, 12 /* add esp, 12*/},
      {0x5F,/*pop edi*/  0x83, 0xC4, 12}, 
      {0x59,/*pop ecx*/  0x83, 0xC4, 12}, 
    };    
    memcpy(b,tab[wv],GLUE_POP_PX_SIZE);
  }

  #define GLUE_SET_PX_FROM_P1_SIZE 2
  static void GLUE_SET_PX_FROM_P1(void *b, int wv)
  {
    static const unsigned char tab[3][GLUE_SET_PX_FROM_P1_SIZE]={
      {0x90,0x90}, // should never be used! (nopnop)
      {0x89,0xC7}, // mov edi, eax
      {0x89,0xC1}, // mov ecx, eax
    };
    memcpy(b,tab[wv],GLUE_SET_PX_FROM_P1_SIZE);
  }

  #define GLUE_POP_FPSTACK_SIZE 2
  static const unsigned char GLUE_POP_FPSTACK[2] = { 0xDD, 0xD8 }; // fstp st0

  static const unsigned char GLUE_POP_FPSTACK_TOSTACK[] = {
    0x83, 0xEC, 16, // sub esp, 16
    0xDD, 0x1C, 0x24 // fstp qword (%esp)  
  };

  static const unsigned char GLUE_POP_STACK_TO_FPSTACK[] = {
    0xDD, 0x04, 0x24, // fld qword (%esp)
    0x83, 0xC4, 16 //  add esp, 16
  };
 
  static const unsigned char GLUE_POP_FPSTACK_TO_WTP[] = { 
      0xDD, 0x1E, /* fstp qword [esi] */
      0x83, 0xC6, 8, /* add esi, 8 */ 
  };

  #define GLUE_SET_PX_FROM_WTP_SIZE 2
  static void GLUE_SET_PX_FROM_WTP(void *b, int wv)
  {
    static const unsigned char tab[3][GLUE_SET_PX_FROM_WTP_SIZE]={
      {0x89,0xF0}, // mov eax, esi
      {0x89,0xF7}, // mov edi, esi
      {0x89,0xF1}, // mov ecx, esi
    };
    memcpy(b,tab[wv],GLUE_SET_PX_FROM_WTP_SIZE);
  }

  #define GLUE_PUSH_VAL_AT_PX_TO_FPSTACK_SIZE 2
  static void GLUE_PUSH_VAL_AT_PX_TO_FPSTACK(void *b, int wv)
  {
    static const unsigned char tab[3][GLUE_PUSH_VAL_AT_PX_TO_FPSTACK_SIZE]={
      {0xDD,0x00}, // fld qword [eax]
      {0xDD,0x07}, // fld qword [edi]
      {0xDD,0x01}, // fld qword [ecx]
    };
    memcpy(b,tab[wv],GLUE_PUSH_VAL_AT_PX_TO_FPSTACK_SIZE);
  }

#define GLUE_POP_FPSTACK_TO_WTP_TO_PX_SIZE (GLUE_SET_PX_FROM_WTP_SIZE + sizeof(GLUE_POP_FPSTACK_TO_WTP))
static void GLUE_POP_FPSTACK_TO_WTP_TO_PX(unsigned char *buf, int wv)
{
  GLUE_SET_PX_FROM_WTP(buf,wv);
  memcpy(buf + GLUE_SET_PX_FROM_WTP_SIZE,GLUE_POP_FPSTACK_TO_WTP,sizeof(GLUE_POP_FPSTACK_TO_WTP));
};


const static unsigned char  GLUE_RET=0xC3;

static int GLUE_RESET_WTP(unsigned char *out, void *ptr)
{
  if (out)
  {
    *out++ = 0xBE; // mov esi, constant
    memcpy(out,&ptr,sizeof(void *));
    out+=sizeof(void *);
  }
  return 1+sizeof(void *);
}



// for gcc on x86 (msvc should already have _controlfp defined)
#if !defined(_RC_CHOP) && !defined(EEL_NO_CHANGE_FPFLAGS)

  #include <fpu_control.h>
  #define _RC_CHOP _FPU_RC_ZERO
  #define _MCW_RC _FPU_RC_ZERO
  static unsigned int _controlfp(unsigned int val, unsigned int mask)
  {
     unsigned int ret;
     _FPU_GETCW(ret);
     if (mask)
     {
       ret&=~mask;
       ret|=val;
       _FPU_SETCW(ret);
     }
     return ret;
  }

#endif


static void GLUE_CALL_CODE(INT_PTR bp, INT_PTR cp, INT_PTR ramptr) 
{
  #ifdef _MSC_VER
    #ifndef EEL_NO_CHANGE_FPFLAGS
      unsigned int old_v=_controlfp(0,0); 
      _controlfp(_RC_CHOP,_MCW_RC);
    #endif

    __asm
    {
      mov eax, cp
      mov ebx, ramptr
      pushad 
      sub esp, 12
      call eax
      add esp, 12
      popad
    };

    #ifndef EEL_NO_CHANGE_FPFLAGS
      _controlfp(old_v,_MCW_RC);
    #endif
  #else // gcc x86
    #ifndef EEL_NO_CHANGE_FPFLAGS
      unsigned int old_v=_controlfp(0,0); 
      _controlfp(_RC_CHOP,_MCW_RC);
    #endif
    __asm__(
          "pushl %%ebx\n"
          "movl %%ecx, %%ebx\n"
          "pushl %%ebp\n"
          "movl %%esp, %%ebp\n"
          "andl $-16, %%esp\n" // align stack to 16 bytes
          "subl $12, %%esp\n" // call will push 4 bytes on stack, align for that
          "call *%%eax\n"
          "leave\n"
          "popl %%ebx\n"
          ::
          "a" (cp), "c" (ramptr): "%edx","%esi","%edi");
    #ifndef EEL_NO_CHANGE_FPFLAGS
      _controlfp(old_v,_MCW_RC);
    #endif
  #endif //gcc x86
}

static unsigned char *EEL_GLUE_set_immediate(void *_p, const void *newv)
{
  char *p=(char*)_p;
  INT_PTR scan = 0xFEFEFEFE;
  while (*(INT_PTR *)p != scan) p++;
  *(INT_PTR *)p = (INT_PTR)newv;
  return (unsigned char *) (((INT_PTR*)p)+1);
}

#define INT_TO_LECHARS(x) ((x)&0xff),(((x)>>8)&0xff), (((x)>>16)&0xff), (((x)>>24)&0xff)


#define GLUE_INLINE_LOOPS

static const unsigned char GLUE_LOOP_LOADCNT[]={
        0xDB, 0x1E,           //fistp dword [esi]
        0x8B, 0x0E,           // mov ecx, [esi]
        0x81, 0xf9, 1,0,0,0,  // cmp ecx, 1
        0x0F, 0x8C, 0,0,0,0,  // JL <skipptr>
};
static const unsigned char GLUE_LOOP_CLAMPCNT[]={
        0x81, 0xf9, INT_TO_LECHARS(NSEEL_LOOPFUNC_SUPPORT_MAXLEN), // cmp ecx, NSEEL_LOOPFUNC_SUPPORT_MAXLEN
        0x0F, 0x8C, 5,0,0,0,  // JL over-the-mov
        0xB9, INT_TO_LECHARS(NSEEL_LOOPFUNC_SUPPORT_MAXLEN), // mov ecx, NSEEL_LOOPFUNC_SUPPORT_MAXLEN
};
static const unsigned char GLUE_LOOP_BEGIN[]={ 
  0x56, //push esi
  0x51, // push ecx
  0x81, 0xEC, 0x08, 0,0,0, // sub esp, 8
};
static const unsigned char GLUE_LOOP_END[]={ 
  0x81, 0xC4, 0x08, 0,0,0, // add esp, 8
  0x59, //pop ecx
  0x5E, // pop esi
  0x49, // dec ecx
  0x0f, 0x85, 0,0,0,0, // jnz ...
};


static const unsigned char GLUE_WHILE_SETUP[]={
        0xB9, INT_TO_LECHARS(NSEEL_LOOPFUNC_SUPPORT_MAXLEN), // mov ecx, NSEEL_LOOPFUNC_SUPPORT_MAXLEN
};
static const unsigned char GLUE_WHILE_BEGIN[]={ 
  0x56, //push esi
  0x51, // push ecx
  0x81, 0xEC, 0x08, 0,0,0, // sub esp, 8
};
static const unsigned char GLUE_WHILE_END[]={ 
  0x81, 0xC4, 0x08, 0,0,0, // add esp, 8
  0x59, //pop ecx
  0x5E, // pop esi


  0x49, // dec ecx
  0x0f, 0x84, 0,0,0,0,  // jz endpt
};
static const unsigned char GLUE_WHILE_CHECK_RV[] = {
  0x85, 0xC0, // test eax, eax
  0x0F, 0x85, 0,0,0,0 // jnz  looppt
};

static const unsigned char GLUE_SET_P1_Z[] = { 0x29, 0xC0 }; // sub eax, eax
static const unsigned char GLUE_SET_P1_NZ[] = { 0xb0, 0x01 }; // mov al, 1

#define GLUE_HAS_FXCH
static const unsigned char GLUE_FXCH[] = {0xd9, 0xc9};

#define GLUE_HAS_FLDZ
static const unsigned char GLUE_FLDZ[] = {0xd9, 0xee};
#define GLUE_HAS_FLD1
static const unsigned char GLUE_FLD1[] = {0xd9, 0xe8};

static EEL_F negativezeropointfive=-0.5f;
static EEL_F onepointfive=1.5f;
#define GLUE_INVSQRT_NEEDREPL &negativezeropointfive, &onepointfive,


#define GLUE_HAS_NATIVE_TRIGSQRTLOG

#endif
