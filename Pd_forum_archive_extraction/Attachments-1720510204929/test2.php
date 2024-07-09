<?php
// needs php5

header("Content-type: image/png");

$pd_file = 'test1.pd';
$pd_font = 3;
$delim = ";\x0A";
$text_maxwidth = 420;
$lets_width = 7;
$pd_objects = array(
// object (<inlets>,<outlets>,<dynamic_inlets>,<dynamic_outlets>)
'print' =>array('1','0','false','false'),
'adc~' =>array('1','2','false','amount_flags'),
'dac~' =>array('2','0','false','amount_flags'),
);

$fp = fopen($pd_file, 'r');
if (!$fp) {
	echo 'Could not open file somefile.txt';
	exit;
}

$object_num = 0;

$pd_obj_inlets = array();
$pd_obj_outlets = array();
//$pd_obj_inlets[0][0][0] = "obj1_inlet1_posx";
//$pd_obj_inlets[0][0][1] = "obj1_inlet1_posy";
//$pd_obj_inlets[0][1][0] = "obj1_inlet2_posx";
//$pd_obj_inlets[0][1][1] = "obj1_inlet2_posy";
//$pd_obj_inlets[1][0][0] = "obj2_inlet1_posx";
//$pd_obj_inlets[1][0][1] = "obj2_inlet1_posy";
//$pd_obj_inlets[1][1][0] = "obj2_inlet2_posx";
//$pd_obj_inlets[1][1][1] = "obj2_inlet2_posy";


$readsize = true;
while (!feof($fp)){
	$pd_line = stream_get_line($fp, 4096, $delim);
	$pd_line = strtr($pd_line, "\x0A\x0D", " ");
	$pd_line = str_replace((" \;"), ";\n", $pd_line);
	$pd_line = str_replace((" \,"), ",", $pd_line);
	$pd_words = explode(' ', $pd_line);
	$pd_words_num = count($pd_words);
	
	// make a array with object pos for connect
	if ($pd_words[0] == '#X'){
		$pd_objpos_arr[$object_num][0] = $pd_words[2];
		$pd_objpos_arr[$object_num][1] = $pd_words[3];
		$object_num++;
	}
	
	if ($pd_words[0] == '#N' && $pd_words[1] == 'canvas' && $readsize == true) {
		$canvas_x_size = $pd_words[4];
		$canvas_y_size = $pd_words[5];
		$im = @imagecreate($canvas_x_size, $canvas_y_size)
	  			or die("Cannot Initialize new GD image stream");
		$background_color = imagecolorallocate($im, 255, 255, 205);
		$black = imagecolorallocate($im, 0, 0, 0);
		$readsize = false;
	}	
	
	if ($pd_words[0] == '#X' && $pd_words[1] == 'obj') {
		imagedrawPDobj($im, $pd_font, $pd_line, $black, $pd_objects, $lets_width, $object_num);
	}	
	
	if ($pd_words[0] == '#X' && $pd_words[1] == 'text') {
		$text_x_pos = $pd_words[2];
		$text_y_pos = $pd_words[3];
		$tmp_text = "";
		for ($i=4; $i<=$pd_words_num-1; $i++){
			$tmp_text = $tmp_text.$pd_words[$i]." ";
		}
		ImageStringWrap($im, 3, $text_x_pos, $text_y_pos, $tmp_text, $black, $text_maxwidth);
	}
	
	if ($pd_words[0] == '#X' && $pd_words[1] == 'msg') {
		$text_x_pos = $pd_words[2];
		$text_y_pos = $pd_words[3];
		$tmp_text = "";
		for ($i=4; $i<=$pd_words_num-1; $i++){
			$tmp_text = $tmp_text.$pd_words[$i]." ";
		}
		$tmp_text = trim($tmp_text);
		drawPDmsg($im, 3, $text_x_pos, $text_y_pos, $tmp_text, $black, $lets_width);
		ImageStringWrap($im, 3, $text_x_pos, $text_y_pos, $tmp_text, $black, $text_maxwidth);
	}

	if ($pd_words[0] == '#X' && $pd_words[1] == 'connect'){
		$x1 = $pd_objpos_arr[$pd_words[2]][0];
		$y1 = $pd_objpos_arr[$pd_words[2]][1];
		$x2 = $pd_objpos_arr[$pd_words[4]][0];
		$y2 = $pd_objpos_arr[$pd_words[4]][1];
		imageline($im, $x1, $y1, $x2, $y2, $black);
	}



}

imagepng($im);
imagedestroy($im);
fclose($fp);

//////////////////////////////////////////////////////////////
function imagedrawPDobj($image, $font, $pdtext, $color, $objlist, $letw, $onum){
	$tpmx = 3; // text position modifier x
	$tpmy = 1; // text position modifier y
	$osmw = 4; // object size modifier width
	$osmh = 2; // object size modifier height
	$words = explode(' ', $pdtext);
	$words_num = count($words);
	$posx = $words[2];
	$posy = $words[3];
	$obj_name = trim($words[4]);
	$objtext = '';
	for ($i=4; $i<=$words_num-1; $i++){ $objtext .= $words[$i]." "; }
	$objtext = trim($objtext);
	$fontwidth = ImageFontWidth($font);
	$fontheight = ImageFontHeight($font);
	$textwidth = strlen($objtext) * $fontwidth;
	$obj_width = $textwidth+$osmw;
	if ($objlist[$obj_name][0] == '1'){
		$inletm = ceil($letw / 2);
		$inlet1_posx = $posx + $inletm;
		$inlet1_posy = $posy + 1;
		$x1 = $inlet1_posx - $inletm;
		$x2 = $inlet1_posx + $inletm;
		$y1 = $inlet1_posy;
		$pd_obj_inlets[$onum][0][0] = $inlet1_posx;
		$pd_obj_inlets[$onum][0][1] = $inlet1_posy;
		imageline($image, $x1, $y1, $x2, $y1, $color);
	}
	imagerectangle($image, $posx, $posy, $posx+$obj_width, $posy+$fontheight+$osmh, $color);
	imagestring($image, $font, $posx+$tpmx, $posy+$tpmy, $objtext, $color);
}

//////////////////////////////////////////////////////////////////////
function drawPDmsg($image, $font, $x, $y, $text, $color, $lets_width){
	$fontwidth = ImageFontWidth($font);
	$fontheight = ImageFontHeight($font);
	$textwidth = strlen($text) * $fontwidth;
	$x = $x - 2;
	$y = $y - 1;
	$fontheight = $fontheight + 2;
	$textwidth = $textwidth + 2;
	imagepolygon($image,
		array (
					$x, $y,
					$x+$textwidth+4, $y,
					$x+$textwidth, $y+4,
					$x+$textwidth, $y+$fontheight-4,
					$x+$textwidth+4, $y+$fontheight,
					$x, $y+$fontheight
					), 6, $color);
	imageline($image, $x, $y+1, $x+$lets_width, $y+1, $color);
	imageline($image, $x, $y+$fontheight-1, $x+$lets_width, $y+$fontheight-1, $color);
}

function ImageStringWrap($image, $font, $x, $y, $text, $color, $maxwidth){
	$fontwidth = ImageFontWidth($font);
	$fontheight = ImageFontHeight($font);
	if ($maxwidth != NULL) {
		$maxcharsperline = floor($maxwidth / $fontwidth);
		$text = wordwrap($text, $maxcharsperline, "\n", 1);
	}
	$lines = explode("\n", $text);
	while (list($numl, $line) = each($lines)) {
		ImageString($image, $font, $x, $y, trim($line), $color);
		$y += $fontheight;
	}
}
?> 
