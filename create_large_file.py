import os 
with open('1gb.txt', 'wb') as fout:
    fout.write(os.urandom(1000000000))
