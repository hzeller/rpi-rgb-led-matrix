#-*- coding: utf-8 -*-
#If this code works, it was written by Seunghyun Lee(www.bluebaynetworks.co.kr).
#If not, I don't know who wrote it
'''
Number       : 10 pixel height
Character    :  8 pixel height
Total Height : 16 pixel height
'''

import numpy as np
import time
from PIL import Image, ImageDraw

num_0 = (
    (2, 5, 4, 16, 0),
    (4, 7, 2, 16, 1),
    (3, 4, 0, 16, 1),
    (6, 6, 1, 16, 1),
    (5, 1, 4, 14, 0),
    (6, 6, 0, 13, 3),
    (5, 1, 4, 12, 0),
    (5, 1, 0, 11, 0),
    (6, 6, 4, 10, 1),
    (6, 6, 0, 9, 1),
    (5, 1, 1, 8, 1),
    (2, 5, 3, 8, 3))


num_1 = (
    (2, 5, 4, 16, 0),
    (3, 4, 4, 15, 1),
    (3, 4, 5, 13, 3),
    (2, 5, 4, 11, 2),
    (0, 0, 4, 8, 0))


num_2 = (
    (0, 0, 4, 16, 0),
    (3, 4, 0, 16, 1),
    (1, 2, 1, 16, 3),
    (1, 2, 1, 15, 0),
    (3, 4, 1, 12, 2),
    (1, 2, 0, 12, 1),
    (2, 5, 3, 12, 3),
    (0, 0, 4, 10, 0),
    (3, 4, 1, 8, 0),
    (2, 5, 3, 8, 3),
    (1, 2, 0, 8, 1))

num_3 = (
    (1, 2, 3, 16, 3),
    (2, 5, 0, 16, 1),
    (3, 4, 1, 15, 2),
    (0, 0, 4, 14, 0),
    (3, 4, 1, 12, 2),
    (1, 2, 0, 12, 1),
    (3, 4, 5, 12, 3),
    (2, 5, 3, 11, 0),
    (3, 4, 1, 8, 0),
    (1, 2, 0, 8, 1),
    (2, 5, 3, 8, 3))
num_4 = (
    (0, 0, 4, 16, 0),
    (0, 0, 4, 14, 0),
    (3, 4, 1, 12, 0),
    (1, 2, 0, 12, 1),
    (2, 5, 0, 10, 0),
    (2, 5, 3, 12, 3),
    (3, 4, 4, 10, 3),
    (2, 5, 0, 9, 2),
    (3, 4, 5, 10, 1))

num_5 = (
    (0, 0, 0, 16, 0),
    (2, 5, 2, 16, 1),
    (2, 5, 3, 15, 0),
    (3, 4, 5, 16, 1),
    (3, 4, 1, 12, 0),
    (1, 2, 0, 12, 1),
    (2, 5, 3, 12, 3),
    (0, 0, 0, 10, 0),
    (3, 4, 1, 8, 2),
    (1, 2, 0, 8, 1),
    (2, 5, 3, 8, 3))
num_6 = (
    (2, 5, 0, 16, 1),
    (5, 1, 2, 16, 1),
    (6, 6, 0, 15, 3),
    (6, 6, 4, 16, 3),
    (5, 1, 4, 14, 0),
    (3, 4, 1, 12, 2),
    (2, 5, 0, 13, 2),
    (3, 4, 2, 11, 0),
    (0, 0, 0, 10, 0),
    (3, 4, 1, 8, 0),
    (1, 2, 0, 8, 1),
    (2, 5, 3, 8, 3))
num_7 = (
    (0, 0, 4, 16, 0),
    (1, 2, 4, 14, 0),
    (3, 4, 5, 13, 1),
    (2, 5, 4, 11, 2),
    (3, 4, 1, 8, 2),
    (2, 5, 3, 8, 3),
    (1, 2, 0, 8, 1))

num_8 = (
    (3, 4, 1, 16, 0),
    (6, 6, 0, 16, 1),
    (3, 4, 5, 16, 1),
    (1, 2, 2, 15, 3),
    (4, 7, 0, 14, 0),
    (1, 2, 1, 12, 3),
    (6, 6, 4, 13, 1),
    (2, 5, 0, 11, 1),
    (4, 7, 0, 10, 0),
    (4, 7, 4, 11, 0),
    (5, 1, 0, 8, 1),
    (5, 1, 2, 8, 1),
    (1, 2, 4, 9, 2))

num_9 = (
    (0, 0, 0, 16, 0),
    (3, 4, 2, 16, 0),
    (1, 2, 2, 15, 3),
    (1, 2, 4, 15, 2),
    (3, 4, 1, 12, 2),
    (3, 4, 5, 12, 3),
    (5, 1, 0, 12, 0),
    (1, 2, 2, 11, 3),
    (5, 1, 4, 9, 0),
    (6, 6, 0, 10, 1),
    (5, 1, 0, 8, 1),
    (6, 6, 2, 8, 2))

'''
ascii 33 - !
'''

a33 = (
    (0,0,2,16,0),
    (2,5,3,13,2),
    (1,3,1,13,2),
    (7,1,1,10,3),
    (7,6,3,10,0)
)

'''
ascii 34 - "
'''

a34 = (
    (7,0,1,11,3),
    (7,1,4,11,3),
    (7,2,1,10,1),
    (7,5,4,10,1)
)

'''
ascii 35 - #
'''

a35 = (
    (7,0,0,16,2),
    (7,5,4,16,1),
    (0,6,2,15,0),
    (7,3,0,14,2),
    (7,1,4,14,1),
    (6,3,0,12,2),
    (6,4,3,12,2),
    (6,6,0,10,0),
    (6,7,3,10,0)
)

'''
ascii 36 - $
'''

a36 = (
    (2,2,0,16,3),
    (7,0,3,16,1),
    (7,3,4,14,3),
    (1,6,2,13,3),
    (1,7,1,13,1),
    (7,3,0,12,1),
    (7,1,1,10,3),
    (2,2,3,10,1)
)

'''
ascii 37 - %
'''

a37 = (
    (0,6,4,16,0),
    (7,1,0,16,3),
    (7,3,1,15,1),
    (0,0,2,13,0),
    (7,5,3,12,0),
    (7,7,4,10,1),
    (0,2,0,10,0)
)

'''
ascii 38 - &
'''

a38 = (
    (2,1,1,16,1),
    (4,0,4,16,0),
    (5,4,0,15,0),
    (6,7,2,14,3),
    (7,6,1,12,0),
    (5,3,4,12,0),
    (5,1,0,11,0),
    (2,2,2,10,3)
)

'''
ascii 39 - '
'''

a39 = (
    (7,3,2,10,1),
    (7,0,2,11,3)
)

'''
ascii 40 - (
'''

a40 = (
    (5,0,1,16,1),
    (2,6,0,14,0),
    (2,3,0,13,2),
    (4,5,1,10,1)
)

'''
ascii 41 - )
'''

a41 = (
    (4,6,2,16,1),
    (2,0,4,14,0),
    (2,3,4,13,2),
    (5,6,2,10,1)
)

'''
ascii 42 - *
'''

a42 = (
    (4,6,0,14,1),
    (5,5,3,14,1),
    (0,2,1,12,0),
    (0,0,3,12,0),
    (2,1,3,10,1),
    (1,2,0,10,3)
)

'''
ascii 43 - +
'''

a43 = (
    (2,1,2,15,0),
    (2,6,0,14,3),
    (0,3,4,13,0),
    (3,4,0,12,0),
    (0,2,2,11,0)
)

'''
ascii 44 -,
'''

a44 = (
    (7,4,1,16,3),
    (7,0,2,15,2)
)

'''
ascii 45 - -
'''

a45 = (
    (3,1,1,13,0),
    (2,6,3,13,3),
    (1,0,0,13,1)
)


'''
ascii 46 - .
'''

a46 = (
    (0,1,3,16,0)
)

'''
ascii 47 - /
'''

a47 = (
    (4,6,0,16,1),
    (7,0,1,14,3),
    (7,3,2,13,2),
    (7,5,3,11,0),
    (7,7,4,10,1)
)

'''
ascii 48 - 0
'''

a48 = (
    (7,0,2,16,3),
    (6,3,0,16,2),
    (7,4,4,16,1),
    (3,2,0,14,1),
    (6,5,1,14,1),
    (0,1,4,14,0),
    (1,3,3,12,3),
    (6,7,0,11,3),
    (0,2,2,10,0),
    (6,3,4,11,1)
)

'''
ascii 49 - 1
'''

a49 = (
    (0,4,0,16,0),
    (1,2,2,16,1),
    (1,4,3,16,3),
    (7,3,2,14,3),
    (5,1,2,13,0),
    (6,6,0,11,0),
    (7,6,0,10,1),
    (0,0,2,10,0)
)

'''
ascii 50 - 2
'''

a50 = (
    (7,4,0,16,2),
    (1,2,2,16,1),
    (1,6,3,16,3),
    (0,2,0,14,0),
    (1,3,2,13,3),
    (1,2,4,13,2),
    (6,6,3,10,0),
    (7,0,2,10,1),
    (0,6,0,10,0)
)

'''
ascii 51 - 3
'''

a51 = (
    (1,2,1,16,3),
    (1,5,0,16,1),
    (5,4,4,16,0),
    (6,6,3,14,2),
    (0,2,1,13,0),
    (1,1,3,12,3),
    (6,5,4,11,1),
    (1,3,1,10,3),
    (1,3,0,10,1)
)

'''
ascii 52 - 4
'''

a52 = (
    (2,1,4,16,0),
    (3,2,4,15,1),
    (3,3,5,13,3),
    (1,0,1,13,3),
    (1,5,0,13,1),
    (7,1,0,11,3),
    (7,3,0,10,1),
    (2,6,4,11,2)
)

'''
ascii 53 - 5
'''

a53 = (
    (2,3,0,16,1),
    (2,4,1,16,3),
    (0,6,4,16,0),
    (0,7,4,14,0),
    (2,1,1,13,1),
    (2,5,0,13,2),
    (3,2,2,12,0),
    (1,4,0,10,1),
    (1,1,1,10,3),
    (0,0,4,10,0)
)

'''
ascii 54 - 6
'''

a54 = (
    (4,4,1,16,1),
    (4,3,3,16,1),
    (2,3,0,15,0),
    (0,0,4,14,0),
    (3,3,0,14,1),
    (7,4,1,13,2),
    (7,2,3,13,1),
    (6,6,0,11,3),
    (6,6,3,10,0),
    (7,4,2,10,1)
)

'''
ascii 55 - 7
'''

a55 = (
    (0,0,2,16,0),
    (0,5,3,14,0),
    (2,5,4,12,0),
    (2,6,2,11,3),
    (0,0,0,10,0),
    (3,1,2,9,0),
)

'''
ascii 56 - 8
'''

a56 = (
    (5,1,0,16,1),
    (4,4,3,16,1),
    (2,2,0,14,0),
    (1,2,4,14,0),
    (0,5,2,13,0),
    (0,6,0,11,0),
    (0,0,4,11,0),
    (3,1,1,9,0)
)

'''
ascii 57 - 9
'''

a57 = (
    (2,3,4,16,0),
    (2,2,4,15,2),
    (3,7,1,12,0),
    (6,6,4,12,3),
    (7,5,0,11,0),
    (7,4,4,10,1),
    (1,1,1,10,1)
)

'''
ascii 58 - :
'''

a58 = (
    (0,1,2,15,0),
    (0,6,2,11,0)
)

'''
ascii 59 -
'''

a59 = (
    (7,3,1,16,3),
    (7,4,2,15,2),
    (0,2,2,11,0)
)

'''
ascii 60 - <
'''

a60 = (
    (7,6,3,16,0),
    (7,1,1,15,2),
    (0,3,0,13,0),
    (7,2,1,11,3),
    (7,4,3,10,1)
)


'''
ascii 61 - =
'''

a61 = (
    (1,1,2,15,3),
    (1,2,1,15,1),
    (1,3,2,11,3),
    (1,5,1,11,1)
)


'''
ascii 62 - >
'''

a62 = (
    (7,4,1,16,3),
    (7,2,3,15,1),
    (0,5,4,13,0),
    (7,3,3,11,0),
    (7,1,1,10,2)
)

'''
ascii 63 - ?
'''

a63 = (
    (7,1,2,16,3),
    (4,3,2,14,1),
    (2,2,4,12,0),
    (2,6,2,11,3),
    (0,4,0,11,0),
    (3,5,1,9,0)
)


'''
ascii 64 - @
'''

a64 = (
    (6,2,1,16,0),
    (6,1,3,16,2),
    (1,3,0,15,0),
    (1,4,2,13,1),
    (3,5,0,12,1),
    (7,6,1,10,1),
    (6,3,2,10,0),
    (1,2,4,12,2)
)


'''
ascii 65 - A
'''

a65 = (
    (0,2,0,16,0),
    (0,1,4,16,0),
    (5,5,3,14,1),
    (4,2,0,14,1),
    (4,1,4,13,0),
    (5,0,0,13,0),
    (4,3,4,11,0),
    (5,2,0,11,0),
    (0,1,2,10,0)
)


'''
ascii 66 - B
'''

a66 = (
    (2,7,0,16,0),
    (1,6,2,16,3),
    (4,3,4,15,0),
    (2,1,0,15,2),
    (2,4,0,13,3),
    (5,1,3,13,0),
    (0,7,0,11,0),
    (4,0,4,11,0),
    (3,3,0,9,0)
)


'''
ascii 67 - C
'''

a67 = (
    (6,5,1,16,0),
    (6,3,3,16,2),
    (7,1,0,15,3),
    (5,7,0,14,0),
    (7,6,0,12,1),
    (6,3,0,10,0),
    (7,6,2,10,2),
    (7,7,4,10,0)
)

'''
ascii 68 - D
'''

a68 = (
    (3,7,0,16,1),
    (0,2,1,16,0),
    (5,3,3,16,0),
    (6,5,4,14,3),
    (1,1,0,14,2),
    (6,2,4,12,1),
    (7,5,0,11,0),
    (7,3,2,10,3),
    (6,4,0,10,2)
)

'''
ascii 69 - E
'''

a69 = (
    (0,6,4,16,0),
    (3,2,0,16,0),
    (3,3,0,15,0),
    (4,4,0,14,1),
    (2,1,1,13,3),
    (2,6,0,13,2),
    (3,0,0,10,0),
    (3,4,0,9,0),
    (0,2,4,10,0)
)


'''
ascii 70 - F
'''

a70 = (
    (2,1,0,16,0),
    (3,2,0,15,1),
    (1,3,1,13,0),
    (5,4,2,13,1),
    (5,5,0,11,0),
    (0,6,2,10,2),
    (0,4,4,10,0)
)

'''
ascii 71 - G
'''

a71 = (
    (6,7,1,16,0),
    (7,4,3,16,2),
    (4,2,4,15,0),
    (3,1,0,15,1),
    (3,0,1,15,1),
    (0,2,0,11,2),
    (6,6,3,10,0),
    (6,7,1,10,2)
)

'''
ascii 72 - H
'''

a72 = (
    (0,6,0,16,0),
    (1,5,4,16,0),
    (1,2,0,14,0),
    (1,1,4,15,2),
    (3,7,1,13,1),
    (0,3,2,13,2),
    (3,4,4,12,1),
    (3,5,5,12,1),
    (2,1,0,11,2)
)

'''
ascii 73 - I
'''

a73 = (
    (6,5,0,16,0),
    (4,1,2,16,0),
    (0,2,4,16,0),
    (4,7,2,14,0),
    (6,6,2,12,3),
    (0,2,4,10,0),
    (7,1,0,10,0),
    (6,3,1,10,2)
)

'''
ascii 74 - J
'''

a74 = (
    (1,7,1,16,3),
    (6,2,0,16,1),
    (6,3,3,15,3),
    (6,4,3,13,1),
    (6,2,4,11,1),
    (0,6,2,10,0),
    (0,1,0,10,0)
)

'''
ascii 75 - K
'''

a75 = (
    (0,2,4,16,0),
    (3,6,0,16,1),
    (2,2,1,16,2),
    (6,7,3,15,1),
    (0,5,1,13,0),
    (3,4,0,12,1),
    (1,2,1,11,0),
    (7,0,3,11,1),
    (7,1,4,10,2)
)

'''
ascii 76 - L
'''

a76 = (
    (1,3,3,16,3),
    (1,1,2,16,1),
    (0,0,0,16,0),
    (2,4,0,14,0),
    (2,2,0,13,2),
    (0,6,0,10,0)
)

'''
ascii 77 - M
'''

a77 = (
    (1,0,0,16,0),
    (2,1,4,16,0),
    (1,3,0,15,2),
    (2,3,4,15,2),
    (0,5,2,13,0),
    (4,6,0,12,1),
    (5,2,3,12,3),
    (6,2,4,11,3),
    (6,1,0,11,1)
)

'''
ascii 78 - N
'''

a78 = (
    (1,6,0,16,0),
    (2,1,4,16,0),
    (1,3,0,15,2),
    (2,0,4,15,2),
    (4,4,2,13,0),
    (2,6,0,12,0),
    (1,1,4,12,0),
    (1,2,4,11,2),
    (2,4,0,11,2)
)


'''
ascii 79 - O
'''

a79 = (
    (4,1,0,16,0),
    (5,0,4,16,0),
    (0,4,2,16,0),
    (6,4,0,14,3),
    (6,5,4,14,1),
    (6,6,0,12,1),
    (6,2,4,12,3),
    (0,1,3,10,0),
    (0,4,1,10,0)
)


'''
ascii 80 - P
'''

a80 = (
    (0,6,0,16,0),
    (4,2,0,14,1),
    (5,4,2,13,3),
    (7,5,4,12,0),
    (6,0,0,13,1),
    (4,1,0,11,0),
    (7,2,4,11,2),
    (3,4,1,9,0)
)

'''
ascii 81 - Q
'''

a81 = (
    (5,2,3,16,1),
    (4,0,1,15,1),
    (7,3,4,14,0),
    (0,5,0,14,0),
    (6,6,4,13,3),
    (0,3,0,12,0),
    (7,1,4,11,1),
    (6,2,0,10,0),
    (6,5,2,10,2)
)

'''
ascii 82 - R
'''

a82 = (
    (0,2,4,16,0),
    (0,3,0,16,0),
    (2,1,4,14,2),
    (1,0,1,14,1),
    (2,6,0,14,2),
    (7,1,4,11,3),
    (7,0,0,11,3),
    (7,4,0,10,1),
    (2,5,2,10,3)
)

'''
ascii 83 - S
'''

a83 = (
    (6,2,0,16,2),
    (6,7,2,16,0),
    (7,3,4,15,3),
    (0,5,3,14,0),
    (1,6,2,13,1),
    (6,0,0,13,3),
    (5,1,0,11,0),
    (0,1,2,10,0),
    (7,2,4,10,0)
)

'''
ascii 84 - T
'''

a84 = (
    (0,3,2,16,0),
    (3,2,2,14,1),
    (3,1,3,14,1),
    (2,5,2,10,1),
    (2,6,3,10,3),
    (0,1,0,10,0)
)

'''
ascii 85 - U
'''

a85 = (
    (6,1,4,16,1),
    (0,4,2,16,0),
    (6,3,0,16,3),
    (4,5,4,14,0),
    (5,6,0,14,0),
    (6,1,4,12,3),
    (6,0,0,12,1),
    (7,4,4,10,1),
    (7,5,0,10,2)
)

'''
ascii 86 - V
'''

a86 = (
    (6,0,1,16,3),
    (7,1,3,16,1),
    (6,4,3,14,0),
    (1,0,1,14,1),
    (2,6,0,14,2),
    (7,1,4,13,2),
    (7,3,0,11,3),
    (7,4,4,11,0),
    (7,3,4,10,2),
    (7,5,0,10,1)
)


'''
ascii 87 - W
'''

a87 = (
    (2,0,1,16,2),
    (1,1,3,16,2),
    (3,4,5,15,1),
    (3,3,0,15,1),
    (2,1,4,13,2),
    (1,2,0,13,2),
    (0,6,0,10,0),
    (0,4,4,10,0)
)


'''
ascii 88 - X
'''

a88 = (
    (7,6,0,16,1),
    (7,7,4,16,2),
    (3,2,1,14,0),
    (0,5,2,13,0),
    (3,6,1,11,0),
    (7,3,4,10,3),
    (7,1,0,10,0)
)

'''
ascii 89 - Y
'''

a89 = (
    (0,2,2,16,0),
    (1,7,2,14,0),
    (7,6,3,13,1),
    (6,5,3,11,0),
    (7,6,4,10,2),
    (6,2,1,12,1),
    (2,1,0,11,2)
)

'''
ascii 90 - Z
'''

a90 = (
    (2,5,0,16,1),
    (1,3,3,16,3),
    (3,1,1,15,0),
    (0,0,2,14,0),
    (7,6,3,13,2),
    (1,1,4,11,2),
    (4,7,3,11,0),
    (7,6,1,10,3),
    (7,2,0,10,1)
)

'''BGR'''
myRED = (0, 0, 255)
myGREEN = (0, 255, 0)
myBLUE = (255, 73, 48)
myWHITE = (255, 255, 255)
myYELLOW = (0, 255, 255)
myCYAN = (255, 255, 0)
myMAGENTA = (255, 0, 255)
myORANGE = (0, 96, 255)
myBLACK = (45, 45, 45)

myCOLORS = (myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myBLACK)
mynums = (num_0, num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9)

mychars = (a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, a48, a49, a50, \
    a51, a52, a53, a54, a55, a56, a57, a58, a59, a60, a61, a62, a63, a64, a65, a66, a67, a68, a69, a70, \
    a71, a72, a73, a74, a75, a76, a77, a78, a79, a80, a81, a82, a83, a84, a85, a86, a87, a88, a89, a90 )
scale = 20
x_shift = 2
bottom_shift = 1
CHAR_HEIGHT = 16
CHAR_WIDTH = 5
CHAR_KERNING = 2
canvas = None

def get_tetris_num(n):
    return mynums[n]

def get_tetris_char(ascii):
    n = ord(ascii)
    if(n < 33 or n > 90):
        return None
    return mychars[n - 33]

def set_pixel(tcanvas, x, x1, x2, y, y1, y2, color, y_pos):
    ret = False
    height, width, _ = tcanvas.shape
    for i in range(x + x1*scale, x + x2*scale):
        for j in range(y + y1*scale, y + y2*scale):
            # print('j:%d y_pos:%d'%(j, y_pos -1))
            if(i < width and j < height):
                tcanvas[j, i] = color
            if j == (y_pos - 1):
                ret = True
    return ret

'''
Don't make shape, just paint the pixel.
rotate:  %4 => 0 ~ 3
'''
def draw_shape(canvas, x, y, color, shape, rotate, y_pos):
    tcanvas = canvas.copy()
    rot = rotate % 4
    ret1 = False
    ret2 = False
    if shape == 0:  #rantangle
        if rot == 0 or rot == 1 or rot == 2 or rot == 3:
            ret1 = set_pixel(tcanvas, x, 0, 2, y, 0, 2, color, y_pos)
    elif shape == 1:
        if rot == 3:
            ret1 = set_pixel(tcanvas, x, 2, 3, y, 1, 2, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 0, 3, y, 2, 3, color, y_pos)
        elif rot == 0:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 0, 3, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 2, 3, color, y_pos)
        elif rot == 1:
            ret1 = set_pixel(tcanvas, x, 0, 3, y, 1, 2, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 0, 1, y, 2, 3, color, y_pos)
        elif rot == 2:
            ret1 = set_pixel(tcanvas, x, 0, 2, y, 1, 2, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 2, 4, color, y_pos)
    elif shape == 2:
        if rot == 1:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 1, 2, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 0, 3, y, 2, 3, color, y_pos)
        elif rot == 2:
            ret1 = set_pixel(tcanvas, x, 0, 2, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 0, 1, y, 0, 3, color, y_pos)
        elif rot == 3:
            ret1 = set_pixel(tcanvas, x, 0, 3, y, 1, 2, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 2, 3, y, 2, 3, color, y_pos)
        elif rot == 0:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 2, 3, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 0, 3, color, y_pos)

    elif shape == 3:
        if rot == 0 or rot == 2:
            ret1 = set_pixel(tcanvas, x, 0, 4, y, 2, 3, color, y_pos)
        elif rot == 1 or rot == 3:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 0, 4, color, y_pos)
    elif shape == 4:
        if rot == 0:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 0, 2, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 1, 3, color, y_pos)
        elif rot == 1:
            ret1 = set_pixel(tcanvas, x, 1, 3, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 0, 2, y, 1, 2, color, y_pos)
        elif rot == 2:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 0, 2, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 1, 3, color, y_pos)
        elif rot == 3:
            ret1 = set_pixel(tcanvas, x, 1, 3, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 0, 2, y, 1, 2, color, y_pos)

    elif shape == 5:
        if rot == 0:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 1, 3, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 0, 2, color, y_pos)
        elif rot == 1:
            ret1 = set_pixel(tcanvas, x, 0, 2, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 3, y, 1, 2, color, y_pos)
        elif rot == 2:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 1, 3, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 0, 2, color, y_pos)
        elif rot == 3:
            ret1 = set_pixel(tcanvas, x, 0, 2, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 3, y, 1, 2, color, y_pos)

    elif shape == 6:
        if rot == 0:
            ret1 = set_pixel(tcanvas, x, 1, 2, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 0, 3, y, 1, 2, color, y_pos)
        elif rot == 1:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 0, 3, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 1, 2, color, y_pos)
        elif rot == 2:
            ret1 = set_pixel(tcanvas, x, 0, 3, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 1, 2, color, y_pos)
        elif rot == 3:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 1, 2, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 0, 3, color, y_pos)


    elif shape == 7:
        if rot == 0:
            ret1 = set_pixel(tcanvas, x, 0, 1, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 0, 2, y, 1, 2, color, y_pos)
        elif rot == 1:
            ret1 = set_pixel(tcanvas, x, 0, 2, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 0, 1, y, 1, 2, color, y_pos)
        elif rot == 2:
            ret1 = set_pixel(tcanvas, x, 0, 2, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 1, 2, y, 1, 2, color, y_pos)
        elif rot == 3:
            ret1 = set_pixel(tcanvas, x, 1, 2, y, 0, 1, color, y_pos)
            ret2 = set_pixel(tcanvas, x, 0, 2, y, 1, 2, color, y_pos)
    return tcanvas, (ret1 or ret2)


def make_canvas(h, w, color):
    global canvas
    canvas = np.zeros([(h ) * scale,(w ) * scale,3], dtype=np.uint8)
    canvas.fill(color)
    return canvas

def set_scale(s):
    global scale
    scale = s

def set_bottom_shift(s):
    global bottom_shift
    bottom_shift = s

class TetrisChar():
    def __init__(self, xpos, ypos, scale):
        self.xpos = xpos
        self.ypos = ypos
        self.scale = scale
        self.movement_cnt = 0
        self.move_end = False
        self.shape_index = 0    # 0 ~ (shape_num - 1)
        self.__reset_shape()

    def __reset_shape(self):
        self.shape_rot = 0      # rotation count
        self.shape_ypos = 0     # xpos is fixed

    def set_char(self, val):
        self.type = 'ASCII'
        self.val = get_tetris_char(val)
        self.__init_val(val)

    def set_num(self, val):
        self.type = 'NUMBER'
        self.val = get_tetris_num(val)
        self.__init_val(val)

    def __init_val(self, val):
        self.shape_num = len(self.val)
        self.shape_move_end = [False for _ in range(self.shape_num)]

    def animate(self, rcanv,  tcanv, ypos):
        i = self.val[self.shape_index]
        shape = i[0]
        x_pos = i[2] + self.xpos
        y_pos = i[3] + ypos - bottom_shift
        rotation = i[4]
        mycanvas, ret = draw_shape(tcanv, x_pos * scale, self.shape_ypos * scale, myCOLORS[i[1]], shape, self.shape_rot, y_pos * scale)
        # print('rot max:%d rot:%d ret:%d'%(rotation, self.shape_rot, ret))
        if ret == True:
            rcanv, _ = draw_shape(rcanv, x_pos * scale, self.shape_ypos * scale, myCOLORS[i[1]], shape, self.shape_rot, y_pos * scale)
            self.shape_move_end[self.shape_index] = True
            if self.shape_index == (self.shape_num - 1):
                self.move_end = True
            else:
                self.shape_index += 1
            self.__reset_shape()
        else:
            if self.shape_rot < rotation:
                self.shape_rot += 1
            self.shape_ypos += 1

        return  mycanvas, rcanv, ret


class TetrisString():
    def __init__(self, xpos, ypos, str):
        self.xpos = xpos
        self.ypos = ypos
        self.scale = scale
        self.string = []
        index = 0.0
        for ascii in str:
            if ascii == ' ':
                index += 0.5
                continue
            A = TetrisChar(xpos + int((CHAR_WIDTH + CHAR_KERNING)*index), ypos, scale)
            A.set_char(ascii)
            self.string.append(A)
            index += 1.0
    def animate(self, matrix, led_buffer):
        global canvas
        tcanv = canvas.copy()
        rcanv = canvas.copy()

        h, w, c = tcanv.shape
        while True:
            index = 0
            move_end = [False for _ in range(len(self.string))]
            for char in self.string:
                if(char.move_end == False):
                    tcanv, rcanv, ret = char.animate(rcanv, tcanv, char.ypos)
                    if(ret == True):
                        canvas = rcanv.copy()
                move_end[index] = char.move_end
                index += 1
            im_pil = Image.fromarray(tcanv)
            led_buffer.SetImage(im_pil)
            led_buffer = matrix.SwapOnVSync(led_buffer)

            time.sleep(0.05)
            if(all(move_end) == True):
                break
            tcanv = canvas
        return tcanv

if __name__ == '__main__':
    set_scale(10)
    set_bottom_shift(1)
    canvas = make_canvas(CHAR_HEIGHT * 2, (CHAR_WIDTH + CHAR_KERNING) * 10  , 0)
    tcanv = canvas
    shift = x_shift + 4

    tetris_str2 = TetrisString(0, CHAR_HEIGHT, scale, "ABCDEF123")
    tcanv = tetris_str2.animate(canvas, tcanv, CHAR_HEIGHT)

    tetris_str = TetrisString(0, 0, scale, "HELLO WORLD")
    tcanv = tetris_str.animate(canvas, tcanv, 0)



    # A = TetrisChar(0, 0, scale)
    # A.set_char('A')
    # B = TetrisChar((CHAR_WIDTH + CHAR_KERNING), 0, scale)
    # B.set_char('B')
    # C = TetrisChar((CHAR_WIDTH + CHAR_KERNING) * 2, 0, scale)
    # C.set_char('C')
    # while True:
    #     tcanv, rcanv, ret1 = A.animate(canvas, tcanv, 0)
    #     if(ret1 == True):
    #         canvas = rcanv.copy()
    #     tcanv, rcanv, ret2 = B.animate(canvas, tcanv, 0)
    #     if(ret2 == True):
    #         canvas = rcanv.copy()
    #     tcanv, rcanv, ret3 = C.animate(canvas, tcanv, 0)
    #     if(ret3 == True):
    #         canvas = rcanv.copy()
    #     cv2.imshow("Tetris", tcanv)
    #     cv2.waitKey(100)
    #     if A.move_end == True and  B.move_end == True:
    #         break
    #     tcanv = canvas
