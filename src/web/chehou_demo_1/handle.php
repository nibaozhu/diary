<?php
include "handle.class.php";

$h = new handle();

if (isset($_GET["method"])) {
	$h->__set("method", $_GET["method"]);
}

if (isset($_GET["table_name"])) {
	$h->__set("table_name", $_GET["table_name"]);
}

if (isset($_GET["row_limit"])) {
	$h->__set("row_limit", $_GET["row_limit"]);
}

if (isset($_GET["offset"])) {
	$h->__set("offset", $_GET["offset"]);
}

$h->i_want_to_play_a_game();

?>
