/*
THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES INC. ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES INC. BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

YOU ASSUME ANY AND ALL RISK FROM THE USE OF THIS CODE OR SUPPORT FILE.

IT IS THE RESPONSIBILITY OF THE PERSON INTEGRATING THIS CODE INTO AN APPLICATION
TO ENSURE THAT THE RESULTING APPLICATION PERFORMS AS REQUIRED AND IS SAFE.

    Module       : cportabl.h
    Description  : The following macros can be used in C modules
                   to allow portability across toolchains

ATTRIBUTE_INTERRUPT
   If this is available for a compiler, flag that a function is an interrupt
   function. This can allow a compiler do some special handling.
KEEP_VAR(var)
   If this is available for a compiler, flag that "var" should NOT be
   optimised away by the compiler even if the compiler thinks that it is not
   used.
WEAK_PROTO(proto)
   If this is available for a compiler, apply whatever attributes are needed
   to a function prototype ("proto") to flag that the function is a "weak" one.
WEAK_FUNC(func)
   If this is available for a compiler, apply whatever attributes are needed
   to a function definition ("func") to flag that the function is a "weak" one.
VECTOR_SECTION
   A particular setup may have a requirement that the vector table be placed
   in a particular section. This specifies the name of that section
SECTION_PLACE(def,sectionname)
   Place the "def" variable in the named section.
RESET_EXCPT_HNDLR
   A particular setup may have a requirement for a different reset handler.
   This specifies the name of that handler.
ALIGN_VAR
   Align a variable to a specific alignment. Note that some compilers do this
   using pragma which can't be included in a macro expansion.(IAR)

    Date         : 01 October 2009
    Version      : v1.00
    Changelog    : v1.00 Initial
*/

#ifndef __CPORTABL_H__
#define __CPORTABL_H__

//*****************************************************************************
//
// GCC. GNU Compiler.
//
//*****************************************************************************
#ifdef __GNUC__
#define ATTRIBUTE_INTERRUPT            __attribute__((__interrupt__))
#define KEEP_VAR(var)                  var __attribute__((used))
#define WEAK_PROTO(proto)              proto __attribute__((weak))
#define WEAK_FUNC(func)                func
#define VECTOR_SECTION                 ".isr_vector"
#define SECTION_PLACE(def,sectionname) __attribute__ ((section(sectionname))) def
#define RESET_EXCPT_HNDLR ResetISR
#define COMPILER_NAME                  "GNUC"
#define __ASM            __asm  
#define __INLINE         inline 

static __INLINE void __NOP()                      { __ASM volatile ("nop"); }
static __INLINE void __WFI()                      { __ASM volatile ("wfi"); }
static __INLINE void __WFE()                      { __ASM volatile ("wfe"); }

#endif // __GNUC__

//*****************************************************************************
//
// RV-MDK. Realview Compiler
//
//*****************************************************************************
#ifdef __ARMCC_VERSION
#define ATTRIBUTE_INTERRUPT
#define KEEP_VAR(var)                  var __attribute__((used))
#define WEAK_PROTO(proto)              proto __attribute__((weak))
#define WEAK_FUNC(func)                func
#define VECTOR_SECTION                 "RESET"
#define SECTION_PLACE(def,sectionname) __attribute__ ((section(sectionname))) def
#define RESET_EXCPT_HNDLR              __main
#define COMPILER_NAME                  "ARMCC"
#define __ASM            __asm  
#define __INLINE         __inline  

#define __NOP                             __nop
#define __WFI                             __wfi
#define __WFE                             __wfe


#endif // __ARMCC_VERSION

//*****************************************************************************
//
// EW-ARM. IAR ARM Compiler.
//
//*****************************************************************************
#ifdef __ICCARM__
#if defined (__ICCARM__)
  #include <intrinsics.h>                     /* IAR Intrinsics   */
#endif

#define ATTRIBUTE_INTERRUPT
#define KEEP_VAR(var)                  __root var
#define WEAK_PROTO(proto)              __weak proto
#define WEAK_FUNC(func)                __weak func
#define VECTOR_SECTION                 ".intvec"
#define SECTION_PLACE(def,sectionname) def @ sectionname
#define RESET_EXCPT_HNDLR              __iar_program_start
#define COMPILER_NAME                  "ICCARM"
#define __ASM           __asm       
#define __INLINE        inline      
#define __NOP                                     __no_operation()          /*!< no operation intrinsic in IAR Compiler */ 
static __INLINE  void __WFI()                     { __ASM ("wfi"); }
static __INLINE  void __WFE()                     { __ASM ("wfe"); }

#endif // __ICCARM__


#if !defined(ATTRIBUTE_INTERRUPT) || !defined(KEEP_VAR) || !defined(WEAK_PROTO) || !defined(WEAK_FUNC) || !defined(VECTOR_SECTION) || !defined(SECTION_PLACE) || !defined(RESET_EXCPT_HNDLR)
   This compiler is not yet supported This should generate an error
#endif

#endif // __CPORTABL_H__
