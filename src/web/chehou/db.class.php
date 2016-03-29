<?php
class db {
	private $link_identifier;
	private $server;
	private $username;
	private $password;
	private $database_name;

	function __construct($server, $username, $password, $database_name) {
		$this->server = $server;
		$this->username = $username;
		$this->password = $password;
		$this->database_name = $database_name;

		$this->connect();

//		mysql_query("set names gbk");
	}

	function __destruct() {
		mysql_close();
	}

	/* Connecting to MYSQL database. */
	function connect() {
		$this->link_identifier = mysql_connect($this->server, $this->username, $this->password)
			or die (mysql_error());

		mysql_select_db($this->database_name, $this->link_identifier)
			or die (mysql_error());

		return $this->link_identifier;
	}

	/* resource For SELECT, SHOW, DESCRIBE, EXPLAIN
	 * and other statements returning resultset,
	 * mysql_query returns a resource on success, or false on error. */
	function execute_query($query) {
		$all_fetched_row = null;
		$resource = mysql_query($query);
		if ($resource) {
			$result = $resource;
			$the_number_of_row = mysql_num_rows($result);
			if ($the_number_of_row > 0) {
				while ($the_fetched_row = mysql_fetch_array($result)) {
					$all_fetched_row[] = $the_fetched_row;
				}
				return $all_fetched_row;
			}
		} else {
			return false;
		}
	}

	/* For other type of SQL statements, INSERT, UPDATE, DELETE, DROP, etc,
	 * mysql_query returns true on success or false on error. */
	function execute_modify($query) {
		$resource = mysql_query($query);
		if ($resource) {
			$the_number_of_affected_rows = mysql_affected_rows($this->link_identifier);
			return $the_number_of_affected_rows;
		}
	}
}

?>
