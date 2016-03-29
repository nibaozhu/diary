<?php
include "db.class.php";

class handle {
	private $__username;
	private $__password;
	private $__remark;

	private $db0;

	function __construct() {
		/* configure */
		$this->db0 = new db("localhost", "root", "toor", "chehou");

	}

	function __destruct() {  }

	function i_want_to_play_a_game($command) {

		switch ($command) {
			case "select_product":
				$this->select_product($product_id);
				break;
			case "insert_product":
				$this->insert_product($product_id, $product_name);
				break;
			case "update_product":
				$this->update_product($product_name, $product_count, $product_id);
				break;
			case "delete_product":
				$this->delete_product($product_id);
				break;
		}
	}


	function insert_product($product_id, $product_name) {
		/* business */
		$query = "insert t_product(p_id, p_name) values('{$product_id}', '{$product_name}')";

		$the_number_of_row = $this->db0->execute_modify($query);
		if ($the_number_of_row) {

			/* Debug */
			echo "<pre>";
			var_dump($the_number_of_row);
			echo "</pre>";

		} else {
			return false;
		}
	}

	function delete_product($product_id) {
		/* business */
		$query = "delete from t_product where p_id = '{$product_id}'";

		$the_number_of_row = $this->db0->execute_modify($query);
		if ($the_number_of_row) {

			/* Debug */
			echo "<pre>";
			var_dump($the_number_of_row);
			echo "</pre>";

		} else {
			return false;
		}
	}

	function select_product($product_id) {
		/* business */
		$query = "select p_id, p_name, p_owner from t_product";

		$the_number_of_row = $this->db0->execute_query($query);
		if ($the_number_of_row) {

			/* Debug */
			echo "<pre>";
			echo "pid\t|p_name\t|p_owner\n";
			for ($i = 0; $i < count($the_number_of_row); $i++) {
				echo $the_number_of_row[$i]["p_id"], "\t|", $the_number_of_row[$i]["p_name"], "\t|", $the_number_of_row[$i]["p_owner"], "\n";
			}
			echo "</pre>";

		} else {
			return false;
		}
	}

	function update_product($product_name, $product_count, $product_id) {
		/* business */
		$query = "update t_product set p_name = '{$product_name}', p_count = '{$product_count}' where p_id = '{$product_id}'";

		$the_number_of_row = $this->db0->execute_modify($query);
		if ($the_number_of_row) {

			/* Debug */
			echo "<pre>";
			var_dump($the_number_of_row);
			echo "</pre>";

		} else {
			return false;
		}
	}
}


?>
