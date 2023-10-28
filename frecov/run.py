import os
os.system("./frecov-64 ./M5-frecov.img 1>ret.txt")

def parse_file(filename):
    with open(filename, 'r') as file:  
        data = file.readlines()  
        result_dict = {}  
        for line in data:  
            value, key = line.strip().split('  ')  
            result_dict[key] = value  
    # print(result_dict)
    return result_dict
sha1 = parse_file('sha1.txt')
ret = parse_file('ret.txt')
right = 0
sum = 0
for key, value in sha1.items():  
    sum += 1
    if key in ret and ret[key] == value:
        right += 1

print(f'ret:{right} / {sum}')