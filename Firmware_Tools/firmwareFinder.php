<?php

error_reporting(0);

if (isset($argv[1])) {
	$date  = str_replace('.', '-', $argv[1]);
	if (isset($argv[2])) {
		$daysRange = $argv[2];
	} else {
		$daysRange = 3;
	}
	if (isset($argv[3])) {
		$startingID = $argv[3];
	} else {
		$startingID = 0;
	}
	if (isset($argv[4])) {
		$endingID = $argv[4];
	} else {
		$endingID = 2400; // Probably HOURMINUTES so 2400 maximum default
	}
	
	$explodeDate = explode('-', $date);
	$sYear  = $explodeDate[0];
	$sMonth = $explodeDate[1];
	$sDay   = $explodeDate[2];

	$timestamp = strtotime($date);
	$startTime = $timestamp;
	$endTime   = $timestamp + ($daysRange * 86400);
	
	$startTimeParse = explode('-', date('Y-m-d', $startTime));
	$startTimeYear  = $startTimeParse[0];
	$startTimeMonth = $startTimeParse[1];
	$startTimeDay   = $startTimeParse[2];
	
	$endTimeParse = explode('-', date('Y-m-d', $endTime));
	$endTimeYear  = $endTimeParse[0];
	$endTimeMonth = $endTimeParse[1];
	$endTimeDay   = $endTimeParse[2];
	

	for ($year = $startTimeYear; $year <= $endTimeYear; $year++) {
		for ($month = $startTimeMonth; $month <= $endTimeMonth; $month++) {
			for ($day = $endTimeDay; $day >= $startTimeDay; $day--) {
				for ($i = $startingID; $i <= $endingID; $i++) {
					$currentFolder = $year."-".$month."-".$day;
					$currentFile = $sYear.".".$sMonth.".".$sDay.".".str_pad($i, 4, "0", STR_PAD_LEFT);
					
					echo $currentFile." in ".$currentFolder."\n";
					
					$url = "http://112.74.112.110/mnt//downloads//".$currentFolder."//U2W_Update_".$currentFile.".img";

					$streamContext = stream_context_create(
						array('http' =>
							array(
								'timeout' => 5 // 5 seconds
							)
						)
					);
					$firmware = file_get_contents($url, false, $streamContext);
					
					if (strpos($http_response_header[0], '200 OK') !== false) {
						echo $url."\n";
						exit;
					}
				}
			}
		}
	}
} else {
	echo "Usage: firmwareFinder.php YYYY-MM-DD DAYS_RANGE STARTING_ID\n\n";
	echo "DAYS_RANGE = Look inside following days folders\n";
	echo "STARTING_ID = Last 4 digits of firmware image\n\n";
	echo "Example: firmwareFinder.php 2021-01-21 0 1000 2400\nTo search for firmware image from 2021.01.20.1000 up to 2021.01.21.2400 inside 2021-01-20 folder only";
}

?>