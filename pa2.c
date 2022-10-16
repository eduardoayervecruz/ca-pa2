// ----------------------------------------------------------------
// 
//   4190.308 Computer Architecture (Fall 2022)
// 
//   Project #2: SFP16 (16-bit floating point) Adder
// 
//   October 4, 2022
// 
//   Seongyeop Jeong (seongyeop.jeong@snu.ac.kr)
//   Jaehoon Shim (mattjs@snu.ac.kr)
//   IlKueon Kang (kangilkueon@snu.ac.kr)
//   Wookje Han (gksdnrwp@snu.ac.kr)
//   Jinsol Park (jinsolpark@snu.ac.kr)
//   Systems Software & Architecture Laboratory
//   Dept. of Computer Science and Engineering
//   Seoul National University
// 
// ----------------------------------------------------------------
#include <stdio.h>

typedef unsigned short SFP16;
/* Add two SFP16-type numbers and return the result */

// signed short calculateE(SFP16 exp){
//   if(exp == 0){
//     return 1 - BIAS;
//   }else if(exp != 127){
//     return exp - BIAS;
//   }else{
//     return ;
//   }
// }

// CORRECT
//#define sign(x) x >> 15
//#define exp(x) (x << 1) >> 9
//#define mantissa(x) (x << 12) >> 9
#define NAN ((SFP16) 0x7f01)
#define NEG_NAN ((SFP16) 0xff01)
#define BIAS 63

SFP16 sign(x){return x >> 15;}

SFP16 exp(SFP16 x){
  SFP16 e = x << 1;
  e >>= 9;
  return e;
}

SFP16 mantissa(SFP16 x){
  SFP16 m = x << 8;
  m >>= 5;
  return m;
}

SFP16 compare_absolute(SFP16 x, SFP16 y){
    if(exp(x) > exp(y)) return 1;
    else if (exp(x) < exp(y)) return 0;
    else if (mantissa(x) > mantissa(y)) return 1;
    else return 0;
}


SFP16 modifyBit(SFP16 n, SFP16 p, SFP16 b)
{
    SFP16 mask = 1 << p;
    return ((n & ~mask) | (b << p));
}

// Every time there in an operation we should shift left 8
// flag is 0 when the program is executed one time and 1 when executed again
SFP16 shift_right_mantissa(SFP16 mantissa, SFP16 oprd){
  if(oprd == 0) return mantissa;
  else{
    if (mantissa % 2 == 1) mantissa = (mantissa >> 1) + 1;
    else mantissa = (mantissa >> 1);
    return shift_right_mantissa(mantissa, oprd - 1);
  }
}


//

// function for Binary Addition
SFP16 binAddition(SFP16 a, SFP16 b)
{
    SFP16 c; //carry
    while (b != 0) {
        //find carry and shift it left
        c = (a & b) << 1;
        //find the sum
        a = a ^ b;
        b = c;
    }
    return a;
}

// function for Binary Subtraction
SFP16 binSubtracton(SFP16 a, SFP16 b)
{
    SFP16 carry;
    //get 2's compliment of b and add in a
    b = binAddition(~b, 1);

    while (b != 0) {
        //find carry and shift it left
        carry = (a & b) << 1;
        //find the sum
        a = a ^ b;
        b = carry;
    }
    return a;
}









//


// if (op == '<') return mantissa << ((oprd + 8) >> 8);

  // if(oprd == 0) return mantissa;
  // else{
  //   if(!flag){
  //     mantissa = mantissa << 4;
  //     flag++;
  //     // Sometimes mantissa can have 12 bits
  //     oprd += 4;
  //   }
  //   // Shift right 8 + oprd
  //   if(mantissa % 4 == 0) shift_right_mantissa(mantissa >> 1,oprd-1,flag);
  //   else{
  //     mantissa >>= 1;
  //     mantissa = modifyBit(mantissa, 15, 1);
  //     shift_right_mantissa(mantissa, oprd - 1, flag);
  //   }
  // }


// Rounds mantissa 
SFP16 until_first_bit(SFP16 n)
{
    if (n == 0) return 0;
    SFP16 count = 0;
    n = n >> 1;
    while (n != 0) {
        n = n >> 1;
        count++;
        //printf("%hu\n", count);
    }
    return count;
}


// Expected bits represent the amount of bits that the mantissa should have
// SFP16 round_mantissa(SFP16 mantissa, SFP16 expectedBits){
//   // Find the first one
//   SFP16 counting = 0;
//   counting = counter(mantissa);
  
//   // Calculate S
//   mantissa = mantissa << (15 - counting);
//   mantissa = shift_right_mantissa(mantissa, 16 - (expectedBits + 2));
//   //printf("%hu\n", mantissa);
  
//   // Round
//   if(mantissa % 8 == 0) return mantissa >> 2;
//   if(mantissa % 8 == 1) return mantissa >> 2;
//   if(mantissa % 8 == 2) return mantissa >> 2;
//   if(mantissa % 8 == 3) return shift_right_mantissa(mantissa, 2);
//   if(mantissa % 8 == 4) return mantissa >> 2;
//   if(mantissa % 8 == 5) return mantissa >> 2;
//   if(mantissa % 8 == 6) return (shift_right_mantissa(mantissa, 3)) << 1;
//   if(mantissa % 8 == 7) return (mantissa + 1)  >> 2;
// }

// END CORRECT

SFP16 fpadd(SFP16 x, SFP16 y)
{
  // /* TODO */
  SFP16 res = 0;
  //printf("x: %hu\n", x);
  //printf("y: %hu\n", y);

  SFP16 res_sign = 0;
  SFP16 res_exp = 0;
  SFP16 res_mantissa = 0;
  
  // HANDLING SPECIAL

  // NaN
  if (!(exp(x) ^ 0x7F) && (mantissa(x) ^ 0x00)) return NAN;
  if (!(exp(y) ^ 0x7F) && (mantissa(y) ^ 0x00)) return NAN;

  // Inf
  if (!(exp(x) ^ 0x7F)) {
    if(!(exp(y) ^ 0x7F)){
      if (sign(x) ^ sign(y))
          // Inf - Inf or -Inf + Inf
          return NAN;
    }
    else return x;
  }
  else if (!(exp(y) ^ 0x7F)) return y;
  
  // catch zeros
  if(!exp(x) && !mantissa(x)){
    return y;
  }else if(!exp(y) && !mantissa(y)){
    return x;
  }

  //CORRECT UNTIL HERE

  // NORMAL + NORMAL
  // 1. Swapping if |y| bigger
  SFP16 tmp = 0;
  if(!compare_absolute(x,y)){
    tmp = x;
    x = y;
    y = tmp;
  }
  // printf("x: %hu\n", x);
  // printf("y: %hu\n", y);
  // // 2. GRS initialization
  // printf("exp(x): %hu\n", exp(x));
  // printf("exp(y): %hu\n", exp(y));

  
  // Initialization
  SFP16 m_x = 0;
  SFP16 m_y = 0;
  signed short E_x = 0;
  signed short E_y = 0;

  //NORMAL OR DENORMAL
  if(exp(x)){
    m_x = modifyBit(mantissa(x), 11, 1);
    E_x = exp(x) - BIAS;
  }else{
    m_x = mantissa(x);
    E_x = 1 - BIAS;
  } 

  if(exp(y)){
    m_y = modifyBit(mantissa(y), 11, 1);
    E_y = exp(y) - BIAS;
  }else{
    m_y = mantissa(y);
    E_y = 1 - BIAS;
  } 
  // printf("m_x: %hu\n", m_x);
  // printf("m_y: %hu\n", m_y);
  // printf("E_x: %hu\n", E_x);
  //printf("E_y: %hu\n", E_y);
  

  //Shift Right d places
  m_y = shift_right_mantissa(m_y, exp(x) - exp(y));
  E_y += (exp(x) - exp(y));
  //printf("E_x: %hu\n", E_x);
  
  signed short E = E_x;
  // printf("E: %hu\n", E);
  // printf("E_y: %hu\n", E_y);
  // printf("m_y: %hu\n", m_y);
  SFP16 counter = 0;


  // printf("sign_x: %hu\n", sign(x));
  // printf("sign_y: %hu\n", sign(y));






    // OPERATION
    if (!(sign(x) ^ sign(y))) {
      res_sign = sign(x);
      res_mantissa = m_y + m_x;
      
      // res_mantissa is of the shape 10.xxxxx
      if((res_mantissa > 4096)){
        res_mantissa = shift_right_mantissa(res_mantissa, 1);
        counter++;
      }

      


      printf("counter: %hu\n", counter);
      // Adjustment of Round and Sticky bits
      res_mantissa = shift_right_mantissa(res_mantissa, 1);
      E = E + counter;
      //E++;

      //printf("res_mantissa: %hu\n", res_mantissa);

      // 5. ROUNDING
      //SFP16 counter = 0;
      SFP16 unrounded_mantisa = res_mantissa;
      // while(1){
        //res_mantissa = round_mantissa(res_mantissa, 9);

      // Find the first one
      SFP16 counting = until_first_bit(res_mantissa);

      // Calculate S
      res_mantissa = res_mantissa << (15 - counting);
      res_mantissa = shift_right_mantissa(res_mantissa, 16 - (9 + 2));
      //printf("%hu\n", mantissa);
  
      // Round
      if(res_mantissa % 8 == 0) res_mantissa >> 2;
      if(res_mantissa % 8 == 1) res_mantissa >> 2;
      if(res_mantissa % 8 == 2) res_mantissa >> 2;
      if(res_mantissa % 8 == 3) shift_right_mantissa(res_mantissa, 2);
      if(res_mantissa % 8 == 4) res_mantissa >> 2;
      if(res_mantissa % 8 == 5) res_mantissa >> 2;
      if(res_mantissa % 8 == 6) (shift_right_mantissa(res_mantissa, 3)) << 1;
      if(res_mantissa % 8 == 7) (res_mantissa + 1)  >> 2;

      res_mantissa >>= 2;






      //printf("rounded: %hu\n", res_mantissa);
      res_mantissa -= 256;
      E += BIAS;

      res_exp = (SFP16) E;
      //printf("res_exp: %hu", res_exp);
    }
    else {
      E += BIAS;
      res_sign = sign(x);
      printf("m_x: %hu\n", m_x);
      printf("m_y: %hu\n", m_y);

      // ACA ESTA EL ERROR
      res_mantissa = binSubtracton(m_x, m_y);

      SFP16 flag_down = 1;
      if(res_mantissa < 2048){
        flag_down = 0;
      }

      
      printf("res_mantissa: %hu\n", res_mantissa);
      

      // res_mantissa is of the shape 10.xxxxx
      if((res_mantissa > 4096)){
        res_mantissa = shift_right_mantissa(res_mantissa, 1);
        counter++;
      }

      //printf("res_mantissa: %hu\n\n", res_mantissa);
      

      // Adjustment of Round and Sticky bits
      res_mantissa = shift_right_mantissa(res_mantissa, 1);
      //printf("res_mantissa: %hu\n", res_mantissa);
      E += counter;
      printf("E: %hu\n", E);
      //E++;

      //printf("res_mantissa: %hu\n", res_mantissa);

      // 5. ROUNDING
      // SFP16 counter = 0;
      SFP16 unrounded_mantisa = res_mantissa;
      // while(1){
      // res_mantissa = round_mantissa(res_mantissa, 9);

      // Find the first one
      SFP16 counting = until_first_bit(res_mantissa);

      // Calculate S
      res_mantissa = res_mantissa << (15 - counting);
      res_mantissa = shift_right_mantissa(res_mantissa, 16 - (9 + 2));

      printf("\nres_mantissa: %hu\n\n", res_mantissa);
      
      //printf("%hu\n", mantissa);
      if(!flag_down) res_mantissa >>= 1;
      //printf("\nres_mantissa: %hu\n\n", res_mantissa);
      // Round
      if(res_mantissa % 8 == 0) res_mantissa = res_mantissa / 4;
      if(res_mantissa % 8 == 1) res_mantissa = res_mantissa >> 2;
      if(res_mantissa % 8 == 2) res_mantissa = res_mantissa >> 2;
      if(res_mantissa % 8 == 3) res_mantissa = shift_right_mantissa(res_mantissa, 2);
      if(res_mantissa % 8 == 4) res_mantissa = res_mantissa >> 2;
      if(res_mantissa % 8 == 5) res_mantissa = res_mantissa >> 2;
      if(res_mantissa % 8 == 6) res_mantissa = (shift_right_mantissa(res_mantissa, 3)) << 1;
      
      
      if(res_mantissa % 8 ==7) res_mantissa = (res_mantissa + 1) / 4 ;

      //res_mantissa >>= 2;

      //printf("res_mantissa: %hu\n", res_mantissa);

      // if(res_mantissa < 2048){
      //   res_mantissa = res_mantissa * 2;
      // }


      //printf("rounded: %hu\n", res_mantissa);
      // res_mantissa = modifyBit(res_mantissa, 9, 0);
      //E += BIAS;
      res_exp = (SFP16) E;

      printf("\nres_sign: %hu\n", res_sign);
      printf("res_exp: %hu\n", res_exp);
      printf("res_mantissa: %hu\n", res_mantissa);





      //   // this actually is a subtraction

      //       res_sign = sign(x);
      //       res_exp = exp(x);

      //       // subtract and round to 23 bit
      //       // this means making room in our 32bit representation
      //       res_mantissa = (mantissa(x) << 1) - ((mantissa(y) << 1) >> d);
        
      //   // 31
      //   if (res_mantissa << 15)  res_mantissa = ((res_mantissa >> 1) + 1);
      //   else res_mantissa = (res_mantissa >> 1);

      //   // normalize mantissa
      //  SFP16 temp = res_mantissa << 8;
      //   for(SFP16 count = 0; count < 8; ++count) {
      //       // 31
      //       if (!((temp << count) >> 15)) {
      //           res_mantissa <<= ++count; // leading 1, so shift 1 more time
      //           res_exp-= count;
      //           break;
      //       }
      //   }
    }
   res = res_sign << 15 | res_exp << 8 | res_mantissa;
   //printf("result: %x\n", res);
   return res;


}
