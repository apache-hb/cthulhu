signed int _Zlhs[1];
signed int _Zrhs[1];
signed int _Zresult[1];
signed int _Zlhs[1] = { 0 };
signed int _Zrhs[1] = { 0 };
signed int _Zresult[1] = { 0 };
void _Zadd(void);
extern signed int printf(const char *, ...);
void _Zentry(void);
void main(void);
void _Zadd(void)
{
  signed int tmp[1];
  tmp[0] = (_Zlhs[0] + _Zrhs[0]);
  _Zresult[0] = tmp[0];
}
void _Zentry(void)
{
  _Zlhs[0] = 25;
  _Zrhs[0] = 50;
  _Zadd();
  printf("%d\x0a", _Zresult[0]);
}
void main(void)
{
  _Zentry();
}
