// Physical layout

#define ROWS 5
#define COLS 14

// what physical keys were pressed last time we checked them?
uint8_t state_slot;
bool state[2][ROWS * COLS];

uint8_t COLUMN_PINS[COLS] =
{
    PIN_F0,
    PIN_F1,
    PIN_F4,
    PIN_F5,
    PIN_F6,
    PIN_F7,
    PIN_B6,
    PIN_B5,
    PIN_D0,
    PIN_B7,
    PIN_B3,
    PIN_B2,
    PIN_B1,
    PIN_B0
};

uint8_t ROW_PINS[ROWS] =
{
    PIN_D1,
    PIN_D2,
    PIN_D3,
    PIN_C6,
    PIN_C7
};

void select_column(uint8_t col)
{
    for (uint8_t i = 0; i < COLS; ++i)
    {
        digitalWrite(COLUMN_PINS[i], i == col ? LOW : HIGH);
    }
}

void scan_matrix(bool* state)
{
    for (uint8_t c = 0; c < COLS; ++c)
    {
        select_column(c);

        uint16_t bits = 0;

        for (uint8_t r = 0; r < ROWS; ++r)
        {
            uint8_t set = (digitalRead(ROW_PINS[r]) == HIGH);
            bits |= (set << r);
        }

        //Serial.print(0x8000 | bits, BIN);
        //Serial.write("\n");

        for (uint8_t r = 0; r < ROWS; ++r)
        {
            uint8_t i = r * COLS + c;
            state[i] = ((bits >> r) & 0x1);
        }
    }
    //Serial.println("--------");
    //delay(1000);
}


uint8_t diffStates(bool* prevState, bool* currState, uint8_t* diff)
{
    uint8_t count = 0;
    for (uint8_t r = 0; r < ROWS; ++r)
    {
        for (uint8_t c = 0; c < COLS; ++c)
        {
            uint8_t i = r * COLS + c;
            bool isUp = currState[i];
            bool wasUp = prevState[i];

            if (isUp != wasUp)
            {
                diff[count] = i;
                if (wasUp) diff[count] |= 0x80;
                ++count;
            }
        }
    }
    return count;
}


#include "keys.h"

#define LROWS 5
#define LCOLS 14
#define LAYERS 2

#define K(k) KEY_##k
uint8_t layout[LAYERS][LROWS * LCOLS] = {
{
    K(GRAVE),  K(1),    K(2),    K(3),     K(4), K(5),     K(6), K(7),     K(8),    K(9),      K(0),         K(MINUS),      K(EQUAL),  K(BACKSPACE),
    K(TAB),    K(Q),    K(W),    K(E),     K(R), K(T),     K(Y), K(U),     K(I),    K(O),      K(P),         K(LBRACE),     K(RBRACE), K(BACKSLASH),
    K(ESC),    K(A),    K(S),    K(D),     K(F), K(G),     K(H), K(J),     K(K),    K(L),      K(SEMICOLON), K(APOSTROPHE), K(_),      K(ENTER),
    K(LSHIFT), K(_),    K(Z),    K(X),     K(C), K(V),     K(B), K(N),     K(M),    K(COMMA),  K(DOT),       K(SLASH),      K(_),      K(RSHIFT),
    K(FN),     K(LGUI), K(LALT), K(LCTRL), K(_), K(SPACE), K(_), K(RCTRL), K(RALT), K(DELETE), K(LEFT),      K(DOWN),       K(UP),     K(RIGHT)
},
{
    K(_),      K(F1),   K(F2),   K(F3),    K(F4),K(F5),    K(F6),K(F7),    K(F8),   K(F9),     K(F10),       K(F11),        K(F12),    K(_),
    K(_),      K(_),    K(_),    K(_),     K(_), K(_),     K(_), K(_),     K(_),    K(_),      K(_),         K(_),          K(_),      K(_),
    K(_),      K(_),    K(_),    K(_),     K(_), K(_),     K(_), K(_),     K(_),    K(_),      K(_),         K(_),          K(_),      K(_),
    K(LSHIFT), K(_),    K(_),    K(_),     K(_), K(_),     K(_), K(_),     K(_),    K(_),      K(_),         K(_),          K(_),      K(RSHIFT),
    K(FN),     K(LGUI), K(LALT), K(LCTRL), K(_), K(SPACE), K(_), K(RCTRL), K(RALT), K(DELETE), K(LEFT),      K(DOWN),       K(UP),     K(RIGHT)
},
};


#define LAYER_NONE 7
#define LAYER_DEFAULT 0
#define LAYER_FN 1

// which layer each physical button (rc) has been pressed in (LAYER_NONE, LAYER_DEFAULT, LAYER_FN)
// we need this to know which virtual key to release once the physical one is released.
uint8_t layer[ROWS * COLS];

#define KEYS 0xEF
// how many pressed physical keys refers to this virtual key?
uint8_t keyRefs[KEYS];


uint8_t keyq[KEYS];
uint8_t keyq_size = 0;

void introduce(uint8_t k)
{
    keyq[keyq_size] = k;
    ++keyq_size;
}

void promote(uint8_t k)
{
    for (uint8_t i = 0; i < keyq_size; ++i)
    {
        if (keyq[i] == k)
        {
            for (uint8_t j = i; j < keyq_size - 1; ++j)
            {
                keyq[j] = keyq[j + 1];
            }
            keyq[keyq_size - 1] = k;
            return;
        }
    }
    // Serial.write("key is not in keyq\n");
}

void forget(uint8_t k)
{
    for (uint8_t i = 0; i < keyq_size; ++i)
    {
        if (keyq[i] == k)
        {
            for (uint8_t j = i; j < keyq_size - 1; ++j)
            {
                keyq[j] = keyq[j + 1];
            }
            --keyq_size;
            return;
        }
    }
    // Serial.write("key is not in keyq\n");
}

bool is_modifier(uint8_t k)
{
    return k >= KEY_LCTRL && k <= KEY_RGUI;
}

void press(uint8_t k)
{
    if (k == KEY__) return;
    ++keyRefs[k];
    if (!is_modifier(k))
    {
        if (keyRefs[k] == 1) introduce(k);
        else promote(k);
    }
}

void release(uint8_t k)
{
    if (k == KEY__) return;
    if (keyRefs[k] > 0) --keyRefs[k];
    // else Serial.write("incorrect ref decrement\n");

    if (!is_modifier(k))
    {
        if (keyRefs[k] == 0) forget(k);
    }
}

#define MODKEYS 8
#define MODBASE KEY_LCTRL

uint8_t getModState()
{
    uint8_t result = 0;
    for (uint8_t i = 0; i < MODKEYS; ++i)
    {
        if (keyRefs[i + MODBASE] > 0) result |= (1 << i);
    }
    return result;
}

uint8_t currentLayer = LAYER_DEFAULT;
uint8_t fnsPressed = 0;

#define MSGSIZE 8
#define KEYSETBASE 2
#define KEYSETSIZE 6

uint8_t oldMessage[MSGSIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t newMessage[MSGSIZE];


// between scans all keys can change their states, so queue should have size ROWS * COLS
uint8_t diff[ROWS * COLS];

uint8_t layout_row(uint8_t r, uint8_t c)
{
    return r;
}

uint8_t layout_col(uint8_t r, uint8_t c)
{
    return c;
}

void print_bin(uint8_t val)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        Serial.print((val & (1 << (7 - i))) ? '1' : '0');
    }
}

const char hex_chars[17] = "0123456789ABCDEF";

void print_hex(uint8_t val)
{
    Serial.print(hex_chars[val >> 4]);
    Serial.print(hex_chars[val & 0x0F]);
}

void setup()
{
    for (uint8_t i = 0; i < ROWS; ++i)
    {
        pinMode(ROW_PINS[i], INPUT_PULLUP);
    }

    // setup columns and deselect them by defaults
    for (uint8_t i = 0; i < COLS; ++i)
    {
        pinMode(COLUMN_PINS[i], OUTPUT);
        digitalWrite(COLUMN_PINS[i], HIGH);
    }

    for (uint8_t i = 0; i < ROWS * COLS; ++i)
    {
        layer[i] = LAYER_NONE;
    }

    for (uint8_t i = 0; i < KEYS; ++i)
    {
        keyRefs[i] = 0;
    }

    Serial.begin(9600);

    state_slot = 0;
    scan_matrix(state[state_slot]);
    state_slot = 1;
}

void loop()
{
    delayMicroseconds(25 * 1000); // 25ms, 40 scans per second

    uint8_t prev_slot = (state_slot + 1) % 2;
    uint8_t curr_slot = state_slot;
    scan_matrix(state[curr_slot]);
    uint8_t numKeysChanged = diffStates(state[prev_slot], state[curr_slot], diff);
    state_slot = prev_slot;

    for (uint8_t i = 0; i < numKeysChanged; ++i)
    {
        uint8_t hw_rc = (diff[i] & 0x7F); 
        uint8_t hw_r = hw_rc / COLS;
        uint8_t hw_c = hw_rc % COLS;

        uint8_t r = layout_row(hw_r, hw_c);
        uint8_t c = layout_col(hw_r, hw_c);
        uint8_t rc = r * LCOLS + c;

        bool down = (diff[i] & 0x80);

        if (down)
        {
            // if (layer[rc] != LAYER_NONE) Serial.write("key is already pressed in some layer\n");
            layer[rc] = currentLayer;
            uint8_t key = layout[currentLayer][rc];
            if (key == KEY_FN) ++fnsPressed;
            else press(key);
        }
        else
        {
            uint8_t keyLayer = layer[rc]; // what layer it was pressed in?
            // if (keyLayer == LAYER_NONE) Serial.write("key was not pressed yet\n");
            layer[rc] = LAYER_NONE;
            uint8_t key = layout[keyLayer][rc];
            if (key == KEY_FN)
            {
                if (fnsPressed > 0) --fnsPressed;
            }
            else release(key);
        }
    }

    currentLayer = (fnsPressed > 0 ? LAYER_FN : LAYER_DEFAULT);

    uint8_t keysToSend = min(KEYSETSIZE, keyq_size);

    newMessage[0] = getModState();
    newMessage[1] = 0;
    for (uint8_t i = 0; i < keysToSend; ++i)
    {
        newMessage[KEYSETBASE + i] = keyq[keyq_size - 1 - i];
    }
    for (uint8_t i = keysToSend; i < KEYSETSIZE; ++i)
    {
        newMessage[KEYSETBASE + i] = 0;
    }

    bool same = true;
    for (uint8_t i = 0; i < MSGSIZE; ++i)
    {
        if (newMessage[i] != oldMessage[i])
        {
            same = false;
            break;
        }
    }
  
    if (!same)
    {
        print_bin(newMessage[0]);
        Serial.print(" : ");
        for (uint8_t i = 1; i < MSGSIZE; ++i)
        {
            print_hex(newMessage[i]);
            Serial.print(" ");
        }
        Serial.print("\r\n");
        
        
        Keyboard.set_modifier(newMessage[0]);
        
        Keyboard.set_key1(newMessage[2]);
        Keyboard.set_key2(newMessage[3]);
        Keyboard.set_key3(newMessage[4]);
        Keyboard.set_key4(newMessage[5]);
        Keyboard.set_key5(newMessage[6]);
        Keyboard.set_key6(newMessage[7]);
        Keyboard.send_now();
        
        for (uint8_t i = 0; i < MSGSIZE; ++i)
        {
            oldMessage[i] = newMessage[i];
        }
    }
}

