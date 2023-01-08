/*
 * Copyright 2023 CAS—Atlantic (University of New Brunswick, CASA)
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
 
`timescale 1ps/1ps

`define MEM_MAXADDR PPP
`define MEM_MAXDATA 36

// depth and data may need to be splited
module single_port_ram(clk, we, addr, data, out);

    parameter ADDR_WIDTH = `MEM_MAXADDR;
    parameter DATA_WIDTH = 1;

    input clk;
    input we;
    input [ADDR_WIDTH - 1:0] addr;
    input [DATA_WIDTH - 1:0] data;
    
    output reg [DATA_WIDTH - 1:0] out;    

	genvar i;
	generate 
		// split in depth
		if (ADDR_WIDTH > `MEM_MAXADDR)
		begin
			
            wire [ADDR_WIDTH-2:0] new_addr = addr[ADDR_WIDTH-2:0];
            wire [DATA_WIDTH-1:0] out_h, out_l;


            defparam uut_h.ADDR_WIDTH = ADDR_WIDTH-1;
            defparam uut_h.DATA_WIDTH = DATA_WIDTH;
            single_port_ram uut_h (
                .clk(clk), 
                .we(we), 
                .addr(new_addr), 
                .data(data), 
                .out(out_h)
            );

            defparam uut_l.ADDR_WIDTH = ADDR_WIDTH-1;
            defparam uut_l.DATA_WIDTH = DATA_WIDTH;
            single_port_ram uut_l (
                .clk(clk), 
                .we(we), 
                .addr(new_addr), 
                .data(data), 
                .out(out_l)
            );

            reg additional_bit;
            always @(posedge clk) additional_bit <= addr[ADDR_WIDTH-1];
            assign out = (additional_bit) ? out_h : out_l;

        end	else begin
            for (i = 0; i < DATA_WIDTH; i = i + 1) begin:single_bit_data
                singlePortRam uut (
                    .clk(clk), 
                    .we(we), 
                    .addr({ {{`MEM_MAXADDR-ADDR_WIDTH}{1'bx}}, addr[ADDR_WIDTH-1:0] }), 
                    .data(data[i]), 
                    .out(out[i])
                );
            end
        end
    endgenerate

endmodule

(* blackbox *)
module singlePortRam(clk, data, addr, we, out);

    localparam ADDR_WIDTH = `MEM_MAXADDR;
    localparam DATA_WIDTH = 1;

    input clk;
    input we;
    input [ADDR_WIDTH-1:0] addr;
    input [DATA_WIDTH-1:0] data;
    
    output reg [DATA_WIDTH-1:0] out;
    /*
    reg [DATA_WIDTH-1:0] RAM [(1<<ADDR_WIDTH)-1:0];

    always @(posedge clk)
    begin
        if (we)
                RAM[addr] <= data;
                
        out <= RAM[addr];
    end
    */
endmodule

