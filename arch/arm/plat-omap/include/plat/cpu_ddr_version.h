#ifndef _CPU_DDR_VERSION_H_
#define _CPU_DDR_VERSION_H_

#define  CP_OMAP3630_ES_1_2_TWL4030_RCONFIG   1
#define  SMARTPHONE_DDR                       512
#define  ES1_1_DDR_MAX_VALUE                  256

#define PMIC_ES1_2              3 //twl5030_1.2 or twl5034
#define PMIC_ES1_1              1
#define PMIC_ES1_0              0

int get_pm_ic_version(void);

#endif
