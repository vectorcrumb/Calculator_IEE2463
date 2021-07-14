#include "calculator.h"

void drawMajorAxes(uint16_t color) {
    drawFastVLine(TFT_WIDTH / 2, 0, TFT_HEIGHT, color);
    drawFastHLine(0, TFT_HEIGHT / 2, TFT_WIDTH, color);
}

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return color;
}

uint8_t calculateFunctionPixels(uint8_t *y_vals, char *expression, double range) {
    // Variables for expression and evaluation results
    double real_x, real_y, max_y = 0;
    te_variable vars[] = {{"x", &real_x}};
    // Compile expression
    int err = 0;
    te_expr *expr = te_compile(expression, vars, 1, &err);
    if (err) return err;
    // By double passing over all values, we sacrifice speed for memory efficiency
    if (expr) {
        // Iterate once to find maximum value by which to scale
        for (int x = 0; x < 160; x++) {
            // Transform pixel x coordinate to real x coordinate
            real_x = range * ((2.0 * x)/TFT_WIDTH - 1);
            // Evaluate expression
            real_y = te_eval(expr);
            // Take absolute value of computed y value
            real_y = real_y < 0 ? real_y * -1 : real_y;
            // Check if abs(real_y) is larger than a previous large y
            if (real_y > max_y) {
                // Replace maximum y value if it is larger
                max_y = real_y;
            }
        }
        max_y *= 1.2;
        // Iterate again to save scaled values in memory
        for (int x = 0; x < 160; x++) {
            // Transform pixel x to real x and evaluate
            real_x = range * ((2.0 * x)/TFT_WIDTH - 1);
            real_y = te_eval(expr);        
            // Transform real y value to pixel y, cast and store
            y_vals[x] = (uint8_t) ((TFT_HEIGHT/2.0) * (real_y / max_y + 1));
        }
    } else {
        return 1;
    }
    te_free(expr);
    return 0;
}

Node * init_keypad(void){
    Node *head;
    head = (Node*)malloc(sizeof(Node));
    head->next=NULL;
    
	DDRD |= (1<<2)  | (1<<3) | (1<<4) | (1<<5);
	DDRC |=  (1<<0) | (1<<1) | (1<<2) | (1<<3);
	PORTD |= (1<<6) | (1<<7);
	PORTB |= (1<<0) | (1<<1);
	
	PCICR |= (1<<PCIE0)    | (1<<PCIE1)   | (1<<PCIE2);
	PCMSK2 |= (1<<PCINT18) | (1<<PCINT19) | (1<<PCINT20) | (1<<PCINT21) | (1<<PCINT22) | (1<<PCINT23);
	PCMSK1 |= (1<<PCINT8)  | (1<<PCINT9)  | (1<<PCINT10) | (1<<PCINT11);
	PCMSK0 |= (1<<PCINT0)  | (1<<PCINT1);
    
    TIMSK0 = (1 << TOIE0);
    return head;
}

void append(char* val, Node * cabeza){
    Node *current = cabeza;
    while (current->next != NULL){
        current = current -> next;
    }
    current->valor = *val;
    current->next = (Node*)malloc(sizeof(Node));
    current->next->next=NULL;
}

char * decode(Node * cabeza, char count){
    Node * current = cabeza;
    char * string = (char *)malloc(count);
    char * buffer = string;
    while ((current->valor != '=')){
        *buffer = current->valor;
        current = current->next;
        buffer++;
    }
    *buffer = '\0';
    Node *nnode = cabeza->next;
    while ((nnode->next != NULL)){
        Node *last = nnode;
        nnode->valor=0x00;
        nnode = nnode->next;
        if (last != NULL) free(last);
    }
    free(nnode);
    cabeza->next=NULL;
    cabeza->valor = 0x00;
    return string;
}