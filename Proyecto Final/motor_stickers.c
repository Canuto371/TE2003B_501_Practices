#include <MKL25Z4.H>

#define PWM_DUTY 42000   // 70% de 60000
#define PWM_MOD  59999   // 20 ms aprox (50Hz)

void delayMs(int n);

int main(void) {
    // Habilitar reloj
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;
    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;

    // Configurar PTA16 y PTA17 como GPIO
    PORTA->PCR[16] = PORT_PCR_MUX(1);
    PORTA->PCR[17] = PORT_PCR_MUX(1);

    // Configurar como salidas
    PTA->PDDR |= (1 << 16) | (1 << 17);

    // Configurar PTE31 como PWM (TPM0_CH4 -> ALT3)
    PORTE->PCR[31] = PORT_PCR_MUX(3);

    // Seleccionar MCGFLLCLK como fuente del TPM
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

    // Configurar TPM0_CH4 como PWM
    TPM0->SC = 0;
    TPM0->CONTROLS[4].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    TPM0->MOD = PWM_MOD;
    TPM0->CONTROLS[4].CnV = PWM_DUTY;
    TPM0->SC = TPM_SC_CMOD(1) | TPM_SC_PS(3);

    while (1) {
        // Dirección: IN1=0, IN2=1
        PTA->PCOR = (1 << 17);
        PTA->PSOR = (1 << 16);
        delayMs(30);

        // Apagar motor
        PTA->PCOR = (1 << 17) | (1 << 16);
        delayMs(1000);
    }
}

// Delay
void delayMs(int n) {
    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 7000; j++) {}
}
