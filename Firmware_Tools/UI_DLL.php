<?php

# Copyright 2021, Ludwig V. <https://github.com/ludwig-v>
# Thanks for original reverse: Pantman <https://github.com/pantman>

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

# php-gd must be installed

if (!function_exists('imagecreatetruecolor')) {
	echo "Please ensure that gd extension is installed and added inside php.ini\r\n\r\n";
	echo "Windows php.ini example:\r\n";
	echo "extension_dir=\"ext\"\r\nextension=php_gd.dll\r\n\r\n";
	
	exit;
}

if (isset($argv) AND isset($argv[1]) AND $argv[1] == 'pack') {
    $packImages = true;
} else {
    $packImages = false;
}

$currentPath = getcwd();

$d = '/';
if (strpos($currentPath, ":\\") !== false) { // Windows path
	$d = '\\';
}

$pointer = fopen($currentPath.$d."rcvec.dll", 'rb');
if ($pointer === false) {
    exit("Error: Can't open rcvec.dll in current directory (".$currentPath.")");
}

$pointer_ = fopen($currentPath.$d."rc.dll", 'rb');
if ($pointer_ === false) {
    exit("Error: Can't open rc.dll in current directory (".$currentPath.")");
}

if ($packImages) {
    $packOffset = 0;

    recursive_mkdir($currentPath.$d."new", $d);

    $pointer_rc_w = fopen($currentPath.$d."new/rc.dll", 'w+');
    if ($pointer_rc_w === false) {
        exit("Error: Can't create rc.dll in current directory (".$currentPath.$d."new)");
    }

    $pointer_rcvec_w = fopen($currentPath.$d."new/rcvec.dll", 'w+');
    if ($pointer_rcvec_w === false) {
        exit("Error: Can't create rcvec.dll in current directory (".$currentPath.$d."new)");
    }
}

$nImages = 0;
while (!feof($pointer)) {
    $nImages++;

    $data = fread($pointer, 80);
    if (strlen($data) < 80) {
        break;
    }

    $header = unpack("I*", $data);

    $extra_data = "";
    $read = fread($pointer, 1);
    while (!feof($pointer) AND ord($read) > 0) {
        $extra_data .= $read;
        $read = fread($pointer, 1);
    }

    $details = array("size" => $header[2],
                     "width" => $header[10],
                     "height" => $header[11],
                     "offset" => $header[17],
                     "path" => str_replace(array('\\', '/'), $d, $extra_data));

    $extra_data .= $read;
    $padding = (4 - (strlen($details['path']) + 1) % 4) % 4;
    if ($padding > 0) {
        $extra_data .= fread($pointer, $padding);
    }

    if (substr($details['path'], 0, 1) != $d) {
        $details['path'] = $d.$details['path'];
    }

    if ($packImages) {
        if (!is_file($currentPath.$d."ui".$details['path'])) {
            exit("Error: ".$currentPath.$d."ui".$details['path']." does not exist, did you delete a file ?");
        }

        list($width, $height) = getimagesize($currentPath.$d."ui".$details['path']);
        $img = imagecreatefrompng($currentPath.$d."ui".$details['path']);
        imagealphablending($img, false);
        imagesavealpha($img, true);

        $pixels = "";

        for ($y = 0; $y < $height; $y++) {
            for ($x = 0; $x < $width; $x++) {
                $rgb_alpha = imagecolorat($img, $x, $y);
                $a = (255 - ((($rgb_alpha >> 24) << 1) & 0xFF));
                $r = ($rgb_alpha >> 16) & 0xFF;
                $g = ($rgb_alpha >> 8) & 0xFF;
                $b = $rgb_alpha & 0xFF;

                $pixel   = array($b, $g, $r, $a);
                $pixels .= array_pack("C*", $pixel);
            }
        }

        $img_encoded = zlib_encode($pixels, ZLIB_ENCODING_DEFLATE);

        unset($pixels);

        $img_encoded_size = strlen($img_encoded);

        fwrite($pointer_rc_w, $img_encoded);

        $header[2]  = $img_encoded_size; // Rewrite size
        $header[10] = $width;  // Rewrite width
        $header[11] = $height; // Rewrite height
        $header[17] = $packOffset; // Rewrite offset

        $packOffset += $img_encoded_size;

        fwrite($pointer_rcvec_w, array_pack("I*", $header));
        fwrite($pointer_rcvec_w, $extra_data);

        echo "- Packed \"".basename($details['path'])."\" (".$img_encoded_size." B) in new/rc.dll\r\n";
    } else {
        fseek($pointer_, $details['offset']);

        $image_data = fread($pointer_, $details['size']);

        $image_data_decoded = zlib_decode($image_data);

        if ($image_data_decoded === false) {
            break;
        }

        unset($image_data);

        $img = imagecreatetruecolor($details['width'], $details['height']);
        imagealphablending($img, false);
        imagesavealpha($img, true);

        $n = 0;
        for ($y = 0; $y < $details['height']; $y++) {
            for ($x = 0; $x < $details['width']; $x++) {
                $color = imagecolorallocatealpha($img, ord($image_data_decoded[$n + 2]), ord($image_data_decoded[$n + 1]), ord($image_data_decoded[$n + 0]), (127 - (ord($image_data_decoded[$n + 3]) >> 1))); // Alpha has to be converted to 0-127
                imagesetpixel($img, $x, $y, $color);
                $n = $n + 4;
            }
        }

        ob_start(); // Start buffering the output
        imagepng($img);
        $png = ob_get_contents();
        ob_end_clean();

        imagedestroy($img);

        $filePathInfo = pathinfo($currentPath.$d."ui".$details['path']);
        recursive_mkdir($filePathInfo['dirname'], $d);
        file_put_contents($filePathInfo['dirname'].$d.$filePathInfo['basename'], $png);

        echo "- Unpacked \"".basename($details['path'])."\" (".$details["size"]." B) from rc.dll\r\n";
    }
}

if ($packImages) {
    echo "\n\nReplace both rc.dll (data) and rcdvec.dll (index) in ui.tar.gz\r\n\r\n";

    fclose($pointer_rc_w);
    fclose($pointer_rcvec_w);
} else {
    echo "\n\n".$nImages." images have been extracted inside ui/ folder\r\nKeep original filenames and paths to repack after editing images\r\n\r\n";
}

fclose($pointer);

function recursive_mkdir($path, $delimiter = '/', $mode = 0777) {
    $dirs = explode($delimiter, $path);
    $count = count($dirs);
    $path = '';
    for ($i = 0; $i < $count; ++$i) {
		if ($i == 0 AND $delimiter != '/') {
			$path .= $dirs[$i];
		} else {
			$path .= $delimiter . $dirs[$i];
		}
        if (!is_dir($path)) {
            if (!mkdir($path, $mode)) {
                exit("Error: Can't create \"".$path."\" directory, check your rights\r\n");
            }
        }
    }
    return true;
}

function array_pack($format, $array) {
  return call_user_func_array("pack", array_merge(array($format), $array));
}

?>