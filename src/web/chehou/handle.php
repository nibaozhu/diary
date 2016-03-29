<?php
include "handle.class.php";

if (isset($_GET["text_command"])) { $command = $_GET["text_command"]; }
if (isset($_GET["text_username"])) { $username = $_GET["text_username"]; }
if (isset($_GET["text_password"])) { $password = $_GET["text_password"]; }

$h = new handle();

if ($command) {
	$h->i_want_to_play_a_game($command);
} else {
	echo "need command\n";
}

?>
