<?php

# Copyright 2020, Ludwig V. <https://github.com/ludwig-v>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License at <http://www.gnu.org/licenses/> for
# more details.

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

$dictionary = array(146,181,188,166,211,183,68,101,132,30,158,110,150,202,18,120,226,242,78,255,156,213,207,85,94,225,162,179,142,249,236,80,209,214,26,7,138,83,117,203,234,104,65,222,204,75,19,199,189,115,221,34,173,62,235,165,159,212,70,157,106,74,161,228,237,5,49,245,171,240,197,208,61,134,137,69,10,111,35,36,23,205,86,16,155,250,153,145,81,254,151,107,14,95,8,27,147,64,29,72,59,50,246,71,175,231,200,108,251,6,140,93,116,172,44,247,219,73,92,253,160,129,52,105,191,230,196,218,42,48,25,252,91,20,149,141,55,77,54,53,47,128,131,187,3,180,135,194,169,243,190,130,186,32,216,177,178,227,41,58,67,118,154,112,15,114,239,210,232,4,109,143,121,176,182,56,217,238,66,174,1,136,144,248,113,152,51,167,123,127,31,13,148,88,198,57,96,233,119,220,122,193,241,82,224,97,163,99,124,185,206,38,90,0,215,192,244,195,40,89,28,24,39,37,184,102,63,223,21,2,79,98,84,133,45,33,46,201,76,139,60,9,126,103,17,12,11,170,229,125,168,22,100,87,43,164);

if (isset($argv) AND isset($argv[1])) {
    if (strpos($argv[1], '.tar.gz') !== false AND is_file($argv[1])) {
        $pointer = fopen($argv[1], 'rb');
        $pointer_ = fopen(str_replace('.tar.gz', '.img', $argv[1]), 'wb');

        while (!feof($pointer)) {
            $data = fread($pointer, (1024 * 512));

            for ($i = 0; $i < strlen($data); $i++) {
                $data[$i] = chr($dictionary[ord($data[$i])]);
            }

            fwrite($pointer_, $data);
        }

        fclose($pointer);
        fclose($pointer_);
        
        echo str_replace('.tar.gz', '.img', $argv[1]).": READY\r\n";
    } else {
        echo "File ".$argv[1]." does not exist !\r\n";
    }
} else {
    echo "Usage: php U2W_Crypt.php filename.tar.gz\r\n";
}

?>