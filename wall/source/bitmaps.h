uint16_t transparent_color=0x0000;
uint16_t ball_bp[4][4] ={
{ 0x0000, 0x9D76, 0x9D76, 0x0000,  },
{ 0x9D76, 0x9D76, 0x9D76, 0x9D76,  },
{ 0x9D76, 0xCEDF, 0x9D76, 0x9D76,  },
{ 0x0000, 0x9D76, 0x9D76, 0x0000,  },
};
uint16_t back_bp[8][8] ={
{ 0x4147, 0x31E7, 0x39EE, 0x31E7, 0x2106, 0x31E7, 0x31E7, 0x31E7,  },
{ 0x31E7, 0x31E7, 0x31E7, 0x2106, 0x31E7, 0x39EE, 0x31E7, 0x4147,  },
{ 0x31E7, 0x31E7, 0x2106, 0x31E7, 0x39EE, 0x31E7, 0x4147, 0x31E7,  },
{ 0x31E7, 0x2106, 0x31E7, 0x4147, 0x31E7, 0x4147, 0x31E7, 0x2106,  },
{ 0x2106, 0x31E7, 0x31E7, 0x31E7, 0x4147, 0x31E7, 0x31E7, 0x31E7,  },
{ 0x31E7, 0x2106, 0x31E7, 0x31E7, 0x31E7, 0x4147, 0x31E7, 0x4147,  },
{ 0x39EE, 0x31E7, 0x2106, 0x31E7, 0x31E7, 0x31E7, 0x4147, 0x31E7,  },
{ 0x31E7, 0x39EE, 0x31E7, 0x2106, 0x31E7, 0x4147, 0x31E7, 0x4147,  },
};
uint16_t brick_bp[8][16] ={
{ 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0xDB84, 0xDB84, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6,  },
{ 0xDB84, 0xDB84, 0xDB84, 0x61C6, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDD0C, 0xDB84, 0xDB84, 0xDB84, 0x61C6, 0xDB84, 0xDB84, 0x61C6,  },
{ 0xDD0C, 0xDB84, 0xDB84, 0xDB84, 0x61C6, 0xDB84, 0x61C6, 0xDB84, 0xDB84, 0xDD0C, 0x61C6, 0x61C6, 0xDB84, 0xDB84, 0xDB84, 0xDB84,  },
{ 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDD0C, 0x61C6, 0xDB84, 0xDB84, 0xDD0C, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0x61C6,  },
{ 0xDD0C, 0xDB84, 0xDD0C, 0x61C6, 0xDB84, 0x61C6, 0xDB84, 0xDD0C, 0x61C6, 0x61C6, 0xDB84, 0xDB84, 0x61C6, 0xDB84, 0xDD0C, 0x61C6,  },
{ 0xDD0C, 0xDB84, 0xDD0C, 0xDB84, 0xDB84, 0x61C6, 0xDB84, 0xDB84, 0x61C6, 0xDB84, 0xDB84, 0xDB84, 0xDD0C, 0x61C6, 0xDB84, 0x61C6,  },
{ 0xDD0C, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0x61C6, 0xDB84, 0xDD0C, 0xDB84, 0xDB84, 0xDD0C, 0xDB84, 0xDB84, 0xDB84, 0x61C6,  },
{ 0xDD0C, 0xDD0C, 0xDB84, 0xDD0C, 0xDD0C, 0xDD0C, 0xDB84, 0xDD0C, 0xDD0C, 0xDD0C, 0xDD0C, 0xDD0C, 0xDD0C, 0xDB84, 0xDD0C, 0x61C6,  },
};
uint16_t paddle_bp[8][32] ={
{ 0x0000, 0x0000, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x0000, 0x0000,  },
{ 0x0000, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x61C6, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xDB84, 0xA986, 0xA986, 0xA986, 0xA986, 0x61C6, 0x0000,  },
{ 0xDB84, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x61C6, 0x0000, 0x0000, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x0000, 0x0000, 0xDB84, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x61C6,  },
{ 0xDB84, 0xA986, 0xA986, 0xDB84, 0x61C6, 0xA986, 0xA986, 0x61C6, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xDB84, 0xA986, 0xA986, 0xDB84, 0x61C6, 0xA986, 0xA986, 0x61C6,  },
{ 0xDB84, 0xA986, 0xA986, 0xDB84, 0xDB84, 0xA986, 0xA986, 0x61C6, 0xA986, 0xA986, 0xDB84, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0x61C6, 0xA986, 0xA986, 0xDB84, 0xA986, 0xA986, 0xDB84, 0xDB84, 0xA986, 0xA986, 0x61C6,  },
{ 0xDB84, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x61C6, 0xA986, 0xA986, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xA986, 0xA986, 0xA986, 0xDB84, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x61C6,  },
{ 0x0000, 0xDB84, 0xA986, 0xA986, 0xA986, 0xA986, 0x61C6, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xDB84, 0xA986, 0xA986, 0xA986, 0xA986, 0x61C6, 0x0000,  },
{ 0x0000, 0x0000, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0xDB84, 0x0000, 0x0000,  },
};