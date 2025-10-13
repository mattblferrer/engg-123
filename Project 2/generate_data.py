import random

with open('data_1.txt', 'w') as f:
    for i in range(30):
        num = random.randint(0, 1000000000000000000)
        hex_num = format(num, '02x')
        f.write(hex_num + '\n')