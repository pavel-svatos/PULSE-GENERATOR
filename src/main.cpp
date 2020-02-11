#include "Arduino.h"
#include "ardprint.h"

typedef void(*func)(void);
// keep old value after reset, if RAM is not changed
bool is_high_pulse      __attribute__ ((section (".noinit")));
bool is_discharge       __attribute__ ((section (".noinit")));
uint32_t duration       __attribute__ ((section (".noinit")));
uint32_t repetive_time  __attribute__ ((section (".noinit")));
uint32_t is_initialied  __attribute__ ((section (".noinit")));

#define clr_scrn()  Serial.println("\033[2J\033[1;1H")
#define POWER_PIN       4
#define DISCHARGE_PIN   5

String read_serial(void);
inline void set_power_fet(uint8_t state) {
    digitalWrite(POWER_PIN, state);
}

inline void set_discharge_fet(uint8_t state) {
    digitalWrite(DISCHARGE_PIN, state);
}

void init_gpio(void) {
    set_discharge_fet(LOW);
    pinMode(DISCHARGE_PIN, OUTPUT);
    set_power_fet(!is_high_pulse);
    pinMode(POWER_PIN, OUTPUT);
}

void init_var(void){
    // keep old value after reset, if RAM is not changed
    if(is_initialied != 0xAAAAAAAA) {      
        is_high_pulse  = false;
        is_discharge   = false;
        duration       = 0;
        repetive_time  = 0;
        is_initialied  = 0xAAAAAAAA;
    }
}

void top_menu(void) {
    Serial.println(" *** PULSE GENERATOR ***");
    Serial.println("     0. Puls type");
    Serial.println("     1. Duration");
    Serial.println("     2. Discharge");
    Serial.println("     3. Repetition");
    Serial.println("     4. START");
}

void show_setup(void) {
    ardprintf("\r\n Puls: %s | Dur[ms]: %d | Discharge: %s | Repeat time[ms]: %d\r\n",
    is_high_pulse ? "high":"low", 
    duration, 
    is_discharge ? "Y":"N",
    repetive_time);
}

void set_puls_type(void) {
    Serial.println(" *** PULSE TYPE ***");
    Serial.println("     0. low");
    Serial.println("     1. high");
    int tmp = read_serial().toInt();
    if (tmp == 0) {
        is_high_pulse = false;
    } else if (tmp == 1) {
        is_high_pulse = true;
    }
}

void set_discharge(void) {
    Serial.println(" *** DISCHARGE ***");
    Serial.println("     0. inactive");
    Serial.println("     1. active");
    int tmp = read_serial().toInt();
    if (tmp == 0) {
        is_discharge = false;
    } else if (tmp == 1) {
        is_discharge = true;
    }
}

void set_duration(void) {
    Serial.println(" *** PULSE DURATION [ms] ***");
    int tmp = read_serial().toInt();
    if (tmp > 0) {
        duration = tmp;
    }
}

void set_repetition(void) {
    Serial.println(" *** REPETIVE TIME [ms] ***");
    int tmp = read_serial().toInt();
    if (tmp >= 0) {
        repetive_time = tmp;
    }
}

void start_pulse(void) {
    if (!duration) {
        return;
    }
    uint32_t cnt = 0;
    uint32_t duration_us = duration * 1000;
    do {
        clr_scrn();
        Serial.println(" *** PULSE GENERATOR ***");
        Serial.print(" Running... Pulse no: ");
        Serial.println(++cnt);
        if (is_high_pulse) {
            set_discharge_fet(LOW);
            delayMicroseconds(50);
            set_power_fet(HIGH);
            delayMicroseconds(duration_us);
            set_power_fet(LOW);
            if (is_discharge) {
                delayMicroseconds(50);
                set_discharge_fet(HIGH);
            }
        } else {
            set_power_fet(LOW);
            if (is_discharge) {
                delayMicroseconds(50);
                set_discharge_fet(HIGH);
            }
            delayMicroseconds(duration_us);
            set_discharge_fet(LOW);
            delayMicroseconds(50);
            set_power_fet(HIGH);
        }
        delay(repetive_time);
    } while (repetive_time);
}

func top_menu_func[] = {
    set_puls_type,
    set_duration,
    set_discharge,
    set_repetition,
    start_pulse,
};

void setup() {
    init_gpio();
    init_var();
    Serial.begin(115200);
    clr_scrn();
}

void loop() {
    top_menu();
    show_setup();
    String cmd = read_serial();
    clr_scrn();
    if (!cmd.length()) {
        // no command -> again
        return;
    }
    uint8_t i = cmd.toInt();
    if (i < (sizeof(top_menu_func)/sizeof(top_menu_func[0])) ) {
        top_menu_func[i]();
    }
    clr_scrn();
}

String read_serial(void) {
    String str = "";
    int c = -1;
    delay(50);
    while (1) {
        c = Serial.read();
        delay(10);
        if (c < 0) {
            continue;
        }
        if (c == 13) {
            return str;
        } else {
            str += char(c);
            Serial.write(c);
        }
    }
}
