# if defined(OPT_DCTO)
COS(1,MAD_F(0x7fd8878e))
COS(2,MAD_F(0x7f62368f))
COS(3,MAD_F(0x7e9d55fc))
COS(4,MAD_F(0x7d8a5f40))
COS(5,MAD_F(0x7c29fbee))
COS(6,MAD_F(0x7a7d055b))
COS(7,MAD_F(0x78848414))
COS(8,MAD_F(0x7641af3d))
COS(9,MAD_F(0x73b5ebd1))
COS(10,MAD_F(0x70e2cbc6))
COS(11,MAD_F(0x6dca0d14))
COS(12,MAD_F(0x6a6d98a4))
COS(13,MAD_F(0x66cf8120))
COS(14,MAD_F(0x62f201ac))
COS(15,MAD_F(0x5ed77c8a))
COS(16,MAD_F(0x5a82799a))
COS(17,MAD_F(0x55f5a4d2))
COS(18,MAD_F(0x5133cc94))
COS(19,MAD_F(0x4c3fdff4))
COS(20,MAD_F(0x471cece7))
COS(21,MAD_F(0x41ce1e65))
COS(22,MAD_F(0x3c56ba70))
COS(23,MAD_F(0x36ba2014))
COS(24,MAD_F(0x30fbc54d))
COS(25,MAD_F(0x2b1f34eb))
COS(26,MAD_F(0x25280c5e))
COS(27,MAD_F(0x1f19f97b))
COS(28,MAD_F(0x18f8b83c))
COS(29,MAD_F(0x12c8106f))
COS(30,MAD_F(0x0c8bd35e))
COS(31,MAD_F(0x0647d97c)
# else
    COS(1  ,MAD_F(0x0ffb10f2))  /* 0.998795456 */
    COS(2  ,MAD_F(0x0fec46d2))  /* 0.995184727 */
    COS(3  ,MAD_F(0x0fd3aac0))  /* 0.989176510 */
    COS(4  ,MAD_F(0x0fb14be8))  /* 0.980785280 */
    COS(5  ,MAD_F(0x0f853f7e))  /* 0.970031253 */
    COS(6  ,MAD_F(0x0f4fa0ab))  /* 0.956940336 */
    COS(7  ,MAD_F(0x0f109082))  /* 0.941544065 */
    COS(8  ,MAD_F(0x0ec835e8))  /* 0.923879533 */
    COS(9  ,MAD_F(0x0e76bd7a))  /* 0.903989293 */
    COS(10 ,MAD_F(0x0e1c5979))  /* 0.881921264 */
    COS(11 ,MAD_F(0x0db941a3))  /* 0.857728610 */
    COS(12 ,MAD_F(0x0d4db315))  /* 0.831469612 */
    COS(13 ,MAD_F(0x0cd9f024))  /* 0.803207531 */
    COS(14 ,MAD_F(0x0c5e4036))  /* 0.773010453 */
    COS(15 ,MAD_F(0x0bdaef91))  /* 0.740951125 */
    COS(16 ,MAD_F(0x0b504f33))  /* 0.707106781 */
    COS(17 ,MAD_F(0x0abeb49a))  /* 0.671558955 */
    COS(18 ,MAD_F(0x0a267993))  /* 0.634393284 */
    COS(19 ,MAD_F(0x0987fbfe))  /* 0.595699304 */
    COS(20 ,MAD_F(0x08e39d9d))  /* 0.555570233 */
    COS(21 ,MAD_F(0x0839c3cd))  /* 0.514102744 */
    COS(22 ,MAD_F(0x078ad74e))  /* 0.471396737 */
    COS(23 ,MAD_F(0x06d74402))  /* 0.427555093 */
    COS(24 ,MAD_F(0x061f78aa))  /* 0.382683432 */
    COS(25 ,MAD_F(0x0563e69d))  /* 0.336889853 */
    COS(26 ,MAD_F(0x04a5018c))  /* 0.290284677 */
    COS(27 ,MAD_F(0x03e33f2f))  /* 0.242980180 */
    COS(28 ,MAD_F(0x031f1708))  /* 0.195090322 */
    COS(29 ,MAD_F(0x0259020e))  /* 0.146730474 */
    COS(30 ,MAD_F(0x01917a6c))  /* 0.098017140 */
    COS(31 ,MAD_F(0x00c8fb30))  /* 0.049067674 */
#endif


#ifdef ADD_DEFINES
#define costab1 costab[1]
#define costab2 costab[2]
#define costab3 costab[3]
#define costab4 costab[4]
#define costab5 costab[5]
#define costab6 costab[6]
#define costab7 costab[7]
#define costab8 costab[8]
#define costab9 costab[9]
#define costab10 costab[10]

#define costab11 costab[11]
#define costab12 costab[12]
#define costab13 costab[13]
#define costab14 costab[14]
#define costab15 costab[15]
#define costab16 costab[16]
#define costab17 costab[17]
#define costab18 costab[18]
#define costab19 costab[19]
#define costab20 costab[20]
    
#define costab21 costab[21]
#define costab22 costab[22]
#define costab23 costab[23]
#define costab24 costab[24]
#define costab25 costab[25]
#define costab26 costab[26]
#define costab27 costab[27]
#define costab28 costab[28]
#define costab29 costab[29]
#define costab30 costab[30]

#define costab31 costab[31]
#endif
