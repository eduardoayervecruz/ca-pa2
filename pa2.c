// ----------------------------------------------------------------
// 
//   4190.308 Computer Architecture (Fall 2022)
// 
//   Project #2: SFP16 (16-bit ing po) Adder
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

SFP16 E = 0;
SFP16 res_denormal = 0;
SFP16 res_normal = 0;
SFP16 res_zero = 0;

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


SFP16 modifyBit(SFP16 bit_string, SFP16 position, SFP16 new_value)
{
    SFP16 mask = 1 << position;
    return ((new_value << position) | (bit_string & ~mask) );
}

// Every time there in an operation we should shift left 8
// flag is 0 when the program is executed one time and 1 when executed again
SFP16 shift_right_mantissa(SFP16 mantissa, SFP16 oprd){
  if(oprd == 0) return mantissa;
  else{
    if (mantissa % 2 == 1){
      SFP16 or_value = ((mantissa << 15) >> 15) || ((mantissa << 14) >> 15);
      mantissa = (mantissa >> 1);
      mantissa = modifyBit(mantissa, 0, or_value);
    }
    else mantissa = (mantissa >> 1);
    return shift_right_mantissa(mantissa, oprd - 1);
  }
}

// function for Binary Addition
SFP16 bit_addition(SFP16 bit_string_1, SFP16 bit_string_2)
{
    SFP16 carry = 0;
    while (bit_string_2 != 0) {
        carry = (bit_string_1 & bit_string_2) << 1;
        bit_string_1 = bit_string_1 ^ bit_string_2;
        bit_string_2 = carry;
    }
    return bit_string_1;
}

// function for Binary Subtraction
SFP16 bit_substraction(SFP16 bit_string_1, SFP16 bit_string_2)
{
    SFP16 carry = 0;
    bit_string_2 = bit_addition(~bit_string_2, 1);
    while (bit_string_2 != 0) {
        carry = (bit_string_1 & bit_string_2) << 1;
        bit_string_1 = bit_string_1 ^ bit_string_2;
        bit_string_2 = carry;
    }
    return bit_string_1;
}

// Rounds mantissa 
SFP16 until_first_one(SFP16 n)
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
SFP16 round_mantissa(SFP16 mantissa, SFP16 expectedBits){
  // Find the first one
  // expectedBits is 9
  if(!res_denormal){
    SFP16 counting = 0;
    counting = until_first_one(mantissa);
    if(counting == 0) return mantissa;
    // Calculate S
    mantissa = mantissa << (15 - counting);
    //printf("value of 9 bit: %hu", (mantissa >> 10));
    mantissa = shift_right_mantissa(mantissa, 16 - (expectedBits + 2));
    //printf("%hu\n", mantissa);
  }
  // Round
  if(mantissa % 8 == 0) {
    //if(!res_denormal){
    E--;
    //}
    return mantissa >> 2;
  }
  if(mantissa % 8 == 1) return mantissa >> 2;
  if(mantissa % 8 == 2) return mantissa >> 2;
  if(mantissa % 8 == 3) return shift_right_mantissa(mantissa, 2);
  if(mantissa % 8 == 4) return mantissa >> 2;
  if(mantissa % 8 == 5) return mantissa >> 2;
  if(mantissa % 8 == 6) return (shift_right_mantissa(mantissa + 2, 3)) << 1;
  if(mantissa % 8 == 7) {
    // The last three numbers are 0
    // E++;
    return ((mantissa + 1) >> 3) << 1;
  };
}
// IF THE FIRST ONE IS IN THE 9 position, then we stop the calculation
// END CORRECT

SFP16 fpadd(SFP16 x, SFP16 y)
{
  // /* TODO */
  SFP16 res = 0;

  SFP16 res_sign = 0;
  SFP16 res_exp = 0;
  SFP16 res_mantissa = 0;
  
  // HANDLING SPECIAL

  // NaN
  if (!(exp(x) ^ 0x7F) && (mantissa(x) ^ 0x00)) return (SFP16) 0x7f01;
  if (!(exp(y) ^ 0x7F) && (mantissa(y) ^ 0x00)) return (SFP16) 0x7f01;

  // Inf
  if (!(exp(x) ^ 0x7F)) {
    if(!(exp(y) ^ 0x7F)){
      if (sign(x) ^ sign(y))
          // Inf - Inf or -Inf + Inf
          return (SFP16) 0x7f01;
    }
    else return x;
  }
  else if (!(exp(y) ^ 0x7F)) {
    if(!(exp(x) ^ 0x7F)){
      if (sign(x) ^ sign(y))
          // Inf - Inf or -Inf + Inf
          return (SFP16) 0x7f01;
    }
    else return y;
  }
  

  // catch zeros
  if(!exp(x) && !mantissa(x)){
    return y;
  }
  if(!exp(y) && !mantissa(y)){
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
  
  
  // 2. GRS initialization
  // printf("exp(x): %hu\n", exp(x));
  // printf("exp(y): %hu\n", exp(y));

  // Initialization
  SFP16 m_x = 0;
  SFP16 m_y = 0;
  SFP16 E_x = 0;
  SFP16 E_y = 0;

  SFP16 x_normal = 0;
  SFP16 x_denormal = 0;
  SFP16 y_normal = 0;
  SFP16 y_denormal = 0;

  //NORMAL OR DENORMAL
  if(exp(x)){
    m_x = modifyBit(mantissa(x), 11, 1);
    E_x = exp(x);
    x_normal = 1;
  }else{
    m_x = mantissa(x);
    E_x = 1;
    x_denormal = 1;
  } 

  if(exp(y)){
    m_y = modifyBit(mantissa(y), 11, 1);
    E_y = exp(y);
    y_normal = 1;
  }else{
    m_y = mantissa(y);
    E_y = 1;
    y_denormal = 1;
  } 
  // printf("m_x: %hu\n", m_x);
  // printf("m_y: %hu\n", m_y);
  // printf("E_x: %hu\n", E_x);
  // printf("E_y: %hu\n", E_y);
  // printf("x_normal: %hu\n", x_normal);
  // printf("y_normal: %hu\n", y_normal);
  
  //Shift Right d places
  m_y = shift_right_mantissa(m_y, E_x - E_y);
  E_y += (exp(x) - exp(y));
  // printf("m_y: %hu\n", m_y);
  // printf("d: %hu\n", exp(x) - exp(y));
  
  E = E_x;
  // printf("E: %hu\n", E);
  // printf("E_y: %hu\n", E_y);
  // printf("m_y: %hu\n", m_y);
  SFP16 counter = 0;


  // printf("sign_x: %hu\n", sign(x));
  // printf("sign_y: %hu\n", sign(y));

    // OPERATION
    // SIGNS ARE THE SAME
    if (!(sign(x) ^ sign(y))){
      if(x_normal && y_normal){
        res_sign = sign(x);
        res_mantissa = m_y + m_x;
        // CATCH ZEROS, INF and NAN
        if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
        if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
        
        
        // printf("m_x: %hu\n", m_x);
        // printf("m_y: %hu\n", m_y);
        // printf("E: %hu\n", E);
        // res_mantissa is of the shape 10.xxxxx
        if(res_mantissa > 4096){
          res_mantissa = (shift_right_mantissa(res_mantissa, 1) << 1);
          E++;
        }
        // printf("res_mantissa: %hu\n", res_mantissa);
        // printf("counter: %hu\n", counter);

        // Adjustment of Round and Sticky bits
        res_mantissa = shift_right_mantissa(res_mantissa, 1);
        // printf("res_mantissa: %hu\n", res_mantissa);
        
        // 5. ROUNDING
        // WHILE THE VALUE DOES NOT HAVE JUST 9 bits
        // while((res_mantissa % 8 == 3) || (res_mantissa % 8 == 6) || (res_mantissa % 8 == 7)){
          res_mantissa = round_mantissa(res_mantissa, 9);
          if(res_mantissa > 4096){
            res_mantissa = shift_right_mantissa(res_mantissa, 1);
            E++;
            //break;
          }
        //}
        // printf("rounded: %hu\n", res_mantissa);
        // Eliminate first one
        res_mantissa -= 256;
        res_exp = (SFP16) E;
        // if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
        // if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
        // printf("res_exp: %hu", res_exp);
      }
      else if (x_normal && y_denormal){
        res_sign = sign(x);
        res_mantissa = m_y + m_x;
        if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
        if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
        // if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
        // printf("m_x: %hu\n", m_x);
        // printf("m_y: %hu\n", m_y);
        // res_mantissa is of the shape 10.xxxxx
        // printf("res_mantissa: %hu\n", res_mantissa);
        if((res_mantissa > 4096)){
          res_mantissa = shift_right_mantissa(res_mantissa, 1);
          E++;
        }
        // There is not enough E to normalize (The result is denormal)
        if ((res_mantissa >> 4) == 0 && 8 > E){
          res_mantissa = res_mantissa << 1;
          E--;
          res_denormal = 1;
        }
        // printf("res_mantissa: %hu\n\n", res_mantissa);
        // Adjustment of Round and Sticky bits
        res_mantissa = shift_right_mantissa(res_mantissa, 1);
        // printf("res_mantissa: %hu\n", res_mantissa);
        
        // 5. ROUNDING
        // WHILE THE VALUE DOES NOT HAVE JUST 9 bits

          res_mantissa = round_mantissa(res_mantissa, 9);
          if(res_mantissa > 4096){
            res_mantissa = shift_right_mantissa(res_mantissa, 1);
            E++;
          }

        //printf("rounded: %hu\n", res_mantissa);
        if(res_mantissa >> 8 == 1){
          res_mantissa -= 256;
        }
        //E += BIAS;

        res_exp = (SFP16) E;
        // if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
        // if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
        //printf("res_exp: %hu", res_exp);
        } // DENORMAL + DENORMAL
        else{
          res_sign = sign(x);
          res_mantissa = m_y + m_x;
          if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
          if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
          // if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
          // printf("m_x: %hu\n", m_x);
          // printf("m_y: %hu\n", m_y);
          // res_mantissa is of the shape 10.xxxxx
          // printf("res_mantissa: %hu\n", res_mantissa);
          if((res_mantissa > 4096)){
            res_mantissa = shift_right_mantissa(res_mantissa, 1);
            E++;
          }
          // printf("E: %hu\n", E);
          // There is not enough E to normalize (The result is denormal)
          if ((res_mantissa >> 4) == 0 && 8 > E ){
            res_mantissa = res_mantissa << 1;

            E--;

            res_denormal = 1;
          }

          // printf("res_mantissa: %hu\n\n", res_mantissa);
          // Adjustment of Round and Sticky bits
          res_mantissa = shift_right_mantissa(res_mantissa, 1);
          // printf("\nres_mantissa: %hu\n", res_mantissa);
          
          // 5. ROUNDING
          // WHILE THE VALUE DOES NOT HAVE JUST 9 bits
            // printf("E: %hu\n", E);
            res_mantissa = round_mantissa(res_mantissa, 9);
            // printf("E: %hu\n", E);
            if(res_mantissa > 4096){
              res_mantissa = shift_right_mantissa(res_mantissa, 1);
              E++;
            }

            //printf("rounded: %hu\n", res_mantissa);
            if(res_mantissa >> 8 == 1){
              res_mantissa -= 256;
              res_normal = 1;
            }
            //E += BIAS;
            // printf("rounded: %hu\n", res_mantissa);
            
            res_exp = E;
            //if(res_exp == 0 && res_mantissa == 0) return 0;
            if(res_exp == 1 && !res_normal) res_exp = 0;
            if(res_exp == 0 && res_mantissa == 0) res_exp = 1;



            // if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
            // if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
            // printf("E: %hu\n", E);
            // printf("res_exp: %hu\n", res_exp);
        }
    }
    // SIGNS ARE DIFFERENT
    else {
      if(x_normal && y_normal){
        res_sign = sign(x);
        // printf("m_x: %hu\n", m_x);
        // printf("m_y: %hu\n", m_y);

        res_mantissa = bit_substraction(m_x, m_y);
        if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
        if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
        // if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
        // printf("res_mantissa: %hu\n", res_mantissa);

        // First bit after the 1. is also 1
        if(((res_mantissa << 11) >> 15) % 2 == 1){
          E--;
          res_mantissa <<= 1;
        }// There is not enough E to normalize (The result is denormal)
        if ((res_mantissa >> 4) == 0 && 8 > E){
          res_mantissa = res_mantissa << 1;
          E--;
          res_denormal = 1;
        }
        // printf("res_mantissa: %hu\n", res_mantissa);
        
        // res_mantissa = res_mantissa << 1;
        // E--;
        // printf("res_mantissa: %hu\n", res_mantissa);
        // Adjustment of Round and Sticky bits
        //if(!res_denormal){
        res_mantissa = shift_right_mantissa(res_mantissa, 1);
        //}
        // //E--;
        // printf("res_mantissa: %hu\n", res_mantissa);

        // 5. ROUNDING

        res_mantissa = round_mantissa(res_mantissa, 9);
        // printf("\nres_mantissa: %hu\n", res_mantissa);
        if((res_mantissa > 511)){
          res_mantissa = shift_right_mantissa(res_mantissa, 1);
          E++;
        }
        if(res_mantissa >> 8 == 1){
          res_mantissa -= 256;
        }
        res_exp = (SFP16) E;

        // if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
        // if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
        // printf("\nres_sign: %hu\n", res_sign);
        // printf("res_exp: %hu\n", res_exp);
        // printf("\nres_mantissa: %hu\n", res_mantissa);
      }else if (x_normal && y_denormal){
        res_sign = sign(x);
        res_mantissa = bit_substraction(m_x, m_y);
        if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
        if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
        // if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
        // printf("m_x: %hu\n", m_x);
        // printf("m_y: %hu\n", m_y);
        // res_mantissa is of the shape 10.xxxxx
        // printf("res_mantissa: %hu\n", res_mantissa);
        if((res_mantissa > 4096)){
          res_mantissa = shift_right_mantissa(res_mantissa, 1);
          E++;
        }
        // There is not enough E to normalize (The result is denormal)
        if ((res_mantissa >> 4) == 0 && 8 > E){
          res_mantissa = res_mantissa << 1;
          E--;
          res_denormal = 1;
        }
        // printf("res_mantissa: %hu\n\n", res_mantissa);
        // Adjustment of Round and Sticky bits
        res_mantissa = shift_right_mantissa(res_mantissa, 1);
        // printf("res_mantissa: %hu\n", res_mantissa);
        
        // 5. ROUNDING
        // WHILE THE VALUE DOES NOT HAVE JUST 9 bits

          res_mantissa = round_mantissa(res_mantissa, 9);
          if(res_mantissa > 4096){
            res_mantissa = shift_right_mantissa(res_mantissa, 1);
            E++;
          }

        // printf("rounded: %hu\n", res_mantissa);
        if(res_mantissa >> 8 == 1){
          res_mantissa -= 256;
        }

        res_exp = (SFP16) E;
        //printf("res_exp: %hu", res_exp);
        }else{
          res_sign = sign(x);
          res_mantissa = bit_substraction(m_x, m_y);
          if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
          if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
          // if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
          printf("m_x: %hu\n", m_x);
          printf("m_y: %hu\n", m_y);
          // res_mantissa is of the shape 10.xxxxx
          printf("res_mantissa: %hu\n", res_mantissa);
          // catch zero
          
          if((res_mantissa > 4096)){
            res_mantissa = shift_right_mantissa(res_mantissa, 1);
            E++;
          }
          // There is not enough E to normalize (The result is denormal)
          if ((res_mantissa >> 4) == 0 && 8 > E){
            res_mantissa = res_mantissa << 1;
            E--;
            res_denormal = 1;
          }
          printf("res_mantissa: %hu\n\n", res_mantissa);
          // Adjustment of Round and Sticky bits
          res_mantissa = shift_right_mantissa(res_mantissa, 1);
          printf("res_mantissa: %hu\n", res_mantissa);
          
          // 5. ROUNDING
          // WHILE THE VALUE DOES NOT HAVE JUST 9 bits

            res_mantissa = round_mantissa(res_mantissa, 9);
            if(res_mantissa > 4096){
              res_mantissa = shift_right_mantissa(res_mantissa, 1);
              E++;
            }

            printf("rounded: %hu\n", res_mantissa);
            if(res_mantissa >> 8 == 1){
              res_mantissa -= 256;
            }
            //E += BIAS;

            res_exp = (SFP16) E;
            
            if(res_exp == 1 && !res_normal) res_exp = 0;
            if(res_exp == 0 && res_mantissa == 0) res_exp = 1;
            // if(((res_sign == 1) ||(res_sign == 0)) && (res_mantissa == 0) && (res_exp == 0)) return 0;
            // if (!(res_exp ^ 0x7F) && (res_mantissa ^ 0x00)) return (SFP16) 0x7f01;
            
            printf("res_exp: %hu", res_exp);
      }
      
    }
   res = res_sign << 15 | res_exp << 8 | res_mantissa;
   //printf("result: %x\n", res);
   return res;


}
