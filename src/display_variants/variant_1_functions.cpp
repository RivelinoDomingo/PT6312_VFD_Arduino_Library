/* PT6312 is an Arduino library for the PT6312 family of Vacuum Fluorescent Display controllers.
 * Copyright (C) 2022 Ysard - <ysard@users.noreply.github.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/* This file is dedicated for a "2 chars per grid display".
 */
#include <global.h>
//#include "HardwareSerial.h"

#ifdef VFD_VARIANT_1

#include "display_variants/variant_1_font.h"
/**
 * @brief Write a string of characters present in the font (If VARIANT_1 is defined in global.h).
 * @param string String must be null terminated '\0'. Grid cursor is auto-incremented.
 *          For this display 6 characters can be displayed simultaneously.
 *          For positions 3 and 4, the grids accept 2 characters.
 *          Positions 1 and 2 accept only 1 char (segments of LSB only),
 *          the other positions are reserved for icons.
 * @param colon_symbol Boolean set to true to display the special colon symbol
 *          segment on grid 4.
 *          The symbol is displayed between chars 4 and 5.
 * @warning The string MUST be null terminated.
 */
void VFD_writeString(const char *string, bool colon_symbol)
{
    // Teste de ordenação
    int index = 0;   // Valor do Indice Completo.
    int H_point=0;   // Onde apareceu o ponto ":"
    bool bt1 = false;
    bool bt2 = false;
    int id = 0;      // Valor de Indice sem o caractere ':'.
    while (string[index] != '\0'){
        //
        if (string[index] == ':'){
           id--;
        }
        index++;
        id++;
    }
    uint8_t txt[id];

    // Limitando o tamanho da exibição aos digitos disponiveis no display.
    if (id > VFD_DISPLAYABLE_DIGITS){
        id = VFD_DISPLAYABLE_DIGITS;
    }
    //
    //// Loop de tratamento de caracteres e de envio de dados para o display.
    for (int i=0; i<id; ++i)
    {
        // Serial.print("\n");
        // Serial.print("  Inicio: ");
        // Serial.print(i);
        // Serial.print(" === ");
        // Serial.print(string[i]);
        // Serial.print("\n");
        // String Exibida "114:03:05"
        /////////////////////////////////
        if (H_point>3 || bt1 || bt2)
        {
            if (bt1)
            {
                txt[i] |= FONT[string[i] - 0x20][1];
                bt1 = false;
            }
            if (bt2)
            {
                txt[i] |= FONT[string[i+2] - 0x20][1];
                bt2 = false;
            }
            if (H_point>3)
            {
                txt[i] = FONT[string[i+2] - 0x20][1]; // Atribui a esse indice o valor de duas casas à frente na *stiring.
            }

        }else if (H_point>0)
        {
            txt[i] = FONT[string[i+1] - 0x20][1]; // Atribui a esse indice o valor do seguinte.
        }else
        {
            txt[i] = FONT[string[i] - 0x20][1];
        }
        /////////////////////////////////
        if (string[i] == ':') // Corrigindo e imprimindo sinal de ':' no display.
        {
            if (i>4) // Condiciona para que imprima os ultimos caracteres da forma certa.
            {
                txt[i-2] |= 0b10000000; // i-2 por que ja apareceu um ponto antes, e deslocou um dos digitos uma casa a frente.
                txt[i-1] = FONT[string[i+1] - 0x20][1]; // Atribui a esse indice o valor uma casa à frente no indice anterior
                H_point = i;
            }else
            {
                txt[i-1] |= 0b10000000;
                txt[i] = FONT[string[i+1] - 0x20][1]; // Atribui a esse indice o valor do seguinte.
                H_point = i;
            }

        }
        /////////////////////////////////
        if (i == id-2) // Corrige erro no segundo digito da 1ª grade.
        {
            if (txt[i] & 0b0000001)
            {
                txt[i+1] = 0b10000000;
                bt1 = true;
            }
            txt[i] = txt[i] >> 1;     // Desloca à direita 1 bit, para corrigir o display.

        }else if (H_point >= id-2)    // Caso o segundo digito tenha sido definido na linha 101.
        {
            txt[i-1] = txt[i-1] >> 1; // Desloca à direita 1 bit, para corrigir o display.
            txt[i] |= 0b10000000;     // Reativa o bit 1 desativado no deslocamento feito acima.
        }
        /////////////////////////////////
        if (i == id-1) // Parte Final.
        {
            // Print for debug.
            // for (int a=0; a<id; ++a)
            // {
            //     Serial.print("  Indice: ");
            //     Serial.print(a);
            //     Serial.print(" = ");
            //     Serial.print(txt[a],BIN);
            // }
            for (int x=id-1; x>=0; x--) // Roda todos os comandos de uma vez.
            {
                VFD_command(txt[x], false);
            }
        }
        /////////////////////////////////
    }
    //Serial.println();

    /*
    while (*string > '\0') { // TODO: security test cursor <= VFD_GRIDS//DISPLAYABLE
        Serial.println(string - 0x20);
        if ((grid_cursor == 1) || (grid_cursor == 2) || (grid_cursor == 3)) {
            // Cursor positions: 3 or 4: 2 chars per grid
            // MSB: Get LSB of left/1st char
            msb_byte = FONT[*string - 0x20][1];
            //Serial.print("MSB_byte: ");
            //Serial.print(msb_byte, BIN);
            //Serial.println();
            string++;
            // Test char validity
            if (*string > '\0') {
                // LSB: Get LSB of right/2nd char
                lsb_byte = FONT[*string - 0x20][1];
                //Serial.print("LSB_byte: ");
                //Serial.println(lsb_byte, BIN);
                //Serial.println();
            } else {
                lsb_byte = 0;
                string--; // Allow end of while loop
            }

            // Corrige erro do meu display!!
            if (grid_cursor == 1){

                if (lsb_byte & 0b00000001){
                    msb_byte |= 0b10000000;
                }
                lsb_byte = lsb_byte >> 1;
            }

            // Set optional colon symbol
            if (colon_symbol && grid_cursor == 1){
                #if VFD_COLON_SYMBOL_BIT > 8
                // Add the symbol on the MSB part of the grid
                msb_byte |= 1 << (VFD_COLON_SYMBOL_BIT - 9);
                #else // < 9
                // Add the symbol on the LSB part of the grid
                lsb_byte |= 1 << (VFD_COLON_SYMBOL_BIT - 1);
                #endif
            }
        }else{
            // Cursor positions: 1 or 2: 1 char only
            // TODO: set only the LSB part to avoid erasing MSB part ?
            // Send LSB
            lsb_byte = FONT[*string - 0x20][1];
            // Send MSB
            msb_byte = FONT[*string - 0x20][0];
        }

        /*
        #if ENABLE_ICON_BUFFER == 1
        // Merge icons and char data
        uint8_t memory_addr = (grid_cursor * PT6312_BYTES_PER_GRID) - PT6312_BYTES_PER_GRID;
        VFD_command(lsb_byte | iconDisplayBuffer[memory_addr], false);
        VFD_command(msb_byte | iconDisplayBuffer[memory_addr + 1], false);
        #else
        VFD_command(lsb_byte, false);
        VFD_command(msb_byte, false);
        #endif


        //VFD_command(msb_byte, false);
        //VFD_command(lsb_byte, false);
        texto_lsb[id] = msb_byte;
        texto_msb[id] = lsb_byte;


        grid_cursor++;
        string++;
        id++;
    }
    */

    /*for (int i=0; i<index; i++){
        VFD_command(texto_msb[index-i], false);
        VFD_command(texto_msb[index-i], false);
    }*/
    // Signal the driver that the data transmission is over
    VFD_CSSignal();
}


/**
 * @brief Animation for a busy spinning circle that uses 1 byte (half grid).
 * @param address Memory address on the controller where the animation frames must be set.
 * @param frame_number Current frame to display (Value range 1..6 (6 segments));
 *      This value is updated when the frame is modified.
 *      The frame number goes back to 1 once 6 is exceeded.
 * @param loop_number Number of refreshes for a frame; used to set the duty cycle of fading frames.
 *      This value is incremented at each call.
 * @note
 *      The animation takes place on the current position set by the value of cursor.
 *      The concerned segments for this display are localized on the grid 1.
 *      The segments are: 11, 12, 13, 14, 15, 16 (it's the MSB part of the grid).
 *
 *      A same frame is refreshed 70 times before moving to the next.
 *      An entire loop is made in 420 calls (6 frames * 70 calls each).
 *      It's up to you to adjust the total time of a loop to 1 second by setting up
 *      a delay (VFD_BUSY_DELAY) after a call (should be ~2.35ms).
 *      The number of refreshes for a frame is stored in loop_number.
 *
 *      A frame is composed of segments displayed at different duty cycles (1, 1/2, 1/5, 1/12)
 *      to obtain a fading effect for the segments behind the main segment.
 *      Ex: For 16th main segment:
 *          15, 14, 13 are displayed, from the most marked to the darkest;
 *          the others are not displayed (12, 11).
 * @warning Since a specific address is used, the grid_cursor global variable IS NOT updated,
 *      and is thus more synchronized with the controller memory.
 *      You SHOULD NOT rely on this value after using this function and use
 *      VFD_setCursorPosition().
 * @see VFD_busyWrapper()
 */
void VFD_busySpinningCircle(uint8_t address, uint8_t& frame_number, uint8_t& loop_number)
{
    uint8_t msb = 0;
    // Init duty cycles divisors
    uint8_t seg2_duty_cycle = 2, seg3_duty_cycle = 5, seg4_duty_cycle = 12;

    // Compute duty cycles triggers
    seg2_duty_cycle = loop_number % seg2_duty_cycle; // 1/2
    seg3_duty_cycle = loop_number % seg3_duty_cycle; // 1/5
    seg4_duty_cycle = loop_number % seg4_duty_cycle; // 1/12

    // Left shifts notes from segment number to bit number:
    // msb: segment number -8 -1
    // lsb: segment number -1
    // Segments successively displayed with 100% of the duty cycle of 1 frame:
    // 11, 12, 13, 14, 15, 16
    // The 3 segments that precede the main displayed segment are fading more and more pronounced.
    if (frame_number == 1) {
        msb = 1 << (11 - 8 - 1);      // segment 11 (first)
    }else if (frame_number == 2) {
        msb = 1 << (12 - 8 - 1);      // segment 12 (second)
        if (seg2_duty_cycle == 0) {
            msb |= 1 << (11 - 8 - 1); // segment 11
        }
    }else if (frame_number == 3) {
        msb = 1 << (13 - 8 - 1);      // segment 13 (third)
        if (seg2_duty_cycle == 0) {
            msb |= 1 << (12 - 8 - 1); // segment 12
        }
        if (seg3_duty_cycle == 0) {
            msb |= 1 << (11 - 8 - 1); // segment 11
        }
    }else if (frame_number == 4) {
        msb = 1 << (14 - 8 - 1);      // segment 14 (fourth)
        if (seg2_duty_cycle == 0) {
            msb |= 1 << (13 - 8 - 1); // segment 13
        }
        if (seg3_duty_cycle == 0) {
            msb |= 1 << (12 - 8 - 1); // segment 12
        }
        if (seg4_duty_cycle == 0) {
            msb |= 1 << (11 - 8 - 1); // segment 11
        }
    }else if (frame_number == 5) {
        msb = 1 << (15 - 8 - 1);      // segment 15 (fifth)
        if (seg2_duty_cycle == 0) {
            msb |= 1 << (14 - 8 - 1); // segment 14
        }
        if (seg3_duty_cycle == 0) {
            msb |= 1 << (13 - 8 - 1); // segment 13
        }
        if (seg4_duty_cycle == 0) {
            msb |= 1 << (12 - 8 - 1); // segment 12
        }
    }else if (frame_number == 6) {
        msb = 1 << (16 - 8 - 1);      // segment 16 (sixth)
        if (seg2_duty_cycle == 0) {
            msb |= 1 << (15 - 8 - 1); // segment 15
        }
        if (seg3_duty_cycle == 0) {
            msb |= 1 << (14 - 8 - 1); // segment 14
        }
        if (seg4_duty_cycle == 0) {
            msb |= 1 << (13 - 8 - 1); // segment 13
        }
    }

    loop_number++;
    if (loop_number == 70) {
        if (frame_number == 6) {
            frame_number = 0;
        }
        frame_number++;
        loop_number = 0;
    }

    #if ENABLE_ICON_BUFFER == 1
    VFD_writeByte(address, msb | iconDisplayBuffer[address]);
    #else
    VFD_writeByte(address, msb);
    #endif

    // If the spinning circle was on 2 bytes, lsb and msb should be sent.
    // Ex:
    // VFD_command(lsb, false);
    // VFD_command(msb, false);
    // VFD_CSSignal();
    // grid_cursor++;

    // Reset/Update display
    // => Don't know why but it appears to be mandatory to avoid forever black screen... (?)
    VFD_resetDisplay();
}

#endif
