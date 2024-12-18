
# Command List
- ADD, CALL, CHK, COPY, DEC, DIG, INC, JNF, JNZ, JMP, LT, LOAD, MOVE, NOP, POP, PUSH, RT, RET, SRT, SWF, SWN, SWP, SUB
- Total: 17

# Instruction Encoding (8 bit words)
- Supports 32 operations: 5 bits to encode instruction
- Additional 3 bits for extra info

# NOP Instruction (8 bits)
- Instruction: 0b00000

# Movement Instruction (8 bits) // SYNC
- Instruction: 0b00001

# Load Constant Instruction (40 bits)
- Instruction: 0b00010
- Register A:  0b000
- Register B:  0b001
- Constant: next 32 bits (4 instructions) (little endian - least significant first)

# Two Register Instruction (8 bits)
- COPY (0b00011) / ADD (0b00100) / SUB (0b00101)
- Register A -> A: 0b000
- Register A -> B: 0b001
- Register B -> A: 0b010
- Register B -> B: 0b011

# Single Register Instruction (8 bits)
- INC (0b00110) / DEC (0b00111) / PUSH (0b01000) / POP (0b01001)
- Register A: 0b000
- Register B: 0b001

# Jump Instruction (24 bits)
- JMP (0b01010) / JNZ (0b01011) / CALL (0b01100) / RET (0b01101) / JNF (0b01110)
- Additional: 0b000
- Address: next 16 bits (2 instructions)

# Turn Instructions (8 bits)
- Left (0b01111) / Right (0b10000)

# Dig Instruction (8 bits) // SYNC
- Instruction: 0b10001

# Check Instruction (8 bits)
- Instruction: 0b10010

# Write Scent Instructions (8 bits)
- Instruction: SWN (0b10011) - scent on // only one scent on at a time
- Scent A: 0b000
- Scent B: 0b001
- Scent C: 0b010
- Scent D: 0b011
- Scent E: 0b100
- Scent F: 0b101
- Scent G: 0b110
- Scent H: 0b111

- Instruction: SWF (0b10100) - all scents off

# Read Scent Instructions (16 bits)
- SWP (0b10101) - set read scent priority
- Scent A: 0b000
- Scent B: 0b001
- Scent C: 0b010
- Scent D: 0b011
- Scent E: 0b100
- Scent F: 0b101
- Scent G: 0b110
- Scent H: 0b111

- Priority: -128 to 127 (8 bits)

- SRT (0b10110) - turn towards scent based on priorities
