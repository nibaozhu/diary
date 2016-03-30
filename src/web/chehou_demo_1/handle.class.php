<?php
include "db.class.php";

class handle {
	private $m_db;

	private $offset;
	private $row_limit;

	private $method;
	private $table_name;

	function __construct() {
		/* configure */
		$this->m_db = new db("localhost", "root", "toor", "chehou");

		$this->offset = 0;
		$this->row_limit = 10;
	}

	function __destruct() {  }

	function i_want_to_play_a_game() {

		switch ($this->method) {

			case "select":
				$this->select_something();
				break;
			case "insert":
				$this->insert_something();
				break;
			case "update":
				$this->update_something();
				break;
			case "delete":
				$this->delete_something();
				break;
		}
	}


	function insert_something($service_id, $service_name) {
		/* business */
		$query = "insert t_service(p_id, p_name) values('{$service_id}', '{$service_name}')";

		$the_number_of_row = $this->m_db->execute_modify($query);
		if ($the_number_of_row) {

			/* Debug */
			echo "<pre>";
			var_dump($the_number_of_row);
			echo "</pre>";

		} else {
			return false;
		}
	}

	function delete_something($service_id) {
		/* business */
		$query = "delete from t_service where p_id = '{$service_id}'";

		$the_number_of_row = $this->m_db->execute_modify($query);
		if ($the_number_of_row) {

			/* Debug */
			echo "<pre>";
			var_dump($the_number_of_row);
			echo "</pre>";

		} else {
			return false;
		}
	}

	function select_something() {
		/* business */

		if ($this->table_name == "t_service_level_0") {
			$query = "select l0_id, l0_name from {$this->table_name} limit {$this->offset}, {$this->row_limit}";
		} else if ($this->table_name == "t_service_level_1") {
			$query = "select l1_id, l1_name, l0_id from {$this->table_name} limit {$this->offset}, {$this->row_limit}";
		} else if ($this->table_name == "t_service_level_2") {
			$query = "select l2_id, l2_name, l1_id from {$this->table_name} limit {$this->offset}, {$this->row_limit}";
		}

		$the_number_of_row = $this->m_db->execute_query($query);
		if ($the_number_of_row) {
			echo "{\"{$this->table_name}\":[";

			if ($this->table_name == "t_service_level_0") {
				for ($i = 0; $i < count($the_number_of_row); $i++) {
					echo 	"{", "\"l0_id\":\"",  $the_number_of_row[$i]["l0_id"],
						"\",\"l0_name\":\"", $the_number_of_row[$i]["l0_name"], "\"}";
					if ($i < count($the_number_of_row) - 1) { echo ","; }
				}
			} else if ($this->table_name == "t_service_level_1") {
				for ($i = 0; $i < count($the_number_of_row); $i++) {
					echo 	"{", "\"l1_id\":\"",  $the_number_of_row[$i]["l1_id"],
						"\",\"l1_name\":\"", $the_number_of_row[$i]["l1_name"],
						"\",\"l0_id\":\"", $the_number_of_row[$i]["l0_id"], "\"}";
					if ($i < count($the_number_of_row) - 1) { echo ","; }
				}
			} else if ($this->table_name == "t_service_level_2") {
				for ($i = 0; $i < count($the_number_of_row); $i++) {
					echo 	"{", "\"l2_id\":\"",  $the_number_of_row[$i]["l2_id"],
						"\",\"l2_name\":\"", $the_number_of_row[$i]["l2_name"],
						"\",\"l1_id\":\"", $the_number_of_row[$i]["l1_id"], "\"}";
					if ($i < count($the_number_of_row) - 1) { echo ","; }
				}
			}

			echo "]}";
		} else {
			return false;
		}
	}

	function update_something($table_name) {
		/* business */
		return false;


		$query = "update {$table_name} set p_name = '{$service_name}', p_count = '{$service_count}' where p_id = '{$service_id}'";

		$the_number_of_row = $this->m_db->execute_modify($query);
		if ($the_number_of_row) {

			/* Debug */
			echo "<pre>";
			var_dump($the_number_of_row);
			echo "</pre>";

		} else {
			return false;
		}
	}


	function __set($variable, $value) {
		if ($variable == "offset") { $this->offset = $value; }
		else if ($variable == "row_limit") { $this->row_limit = $value; }
		else if ($variable == "method") { $this->method = $value; }
		else if ($variable == "table_name") { $this->table_name = $value; }
	}

/*
	function __get($variable) {
		if ($variable == "offset") { return $this->variable; }
		else if ($variable == "offset") { return $this->variable; }
		else if ($variable == "service_id") { return $this->variable; }
	}
*/

}


?>
