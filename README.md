# float16
Float 16/32 Converter
  
Example:  
  
int main()  
{  
  float f32 = 1.2345f;  
  half f16 = ftoh(f32);     // convert float32 to float16, it may cause overflow or loss precision.  
  float f32b = htof(f16);   // convert float16 to float32  
  printf("f32=%f, f32b=%f\r\n", f32, f32b);  
  return 0;  
}  
