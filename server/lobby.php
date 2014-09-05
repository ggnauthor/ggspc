<?php
define("NODE_LIST_FILE", "nodelist.dat");
define("LOCK_DIR_NAME", ".lock");

main();

function main() {
  print("##head##\n");

  //$file = fopen("sample.txt", "w");
  if ($_SERVER['REQUEST_METHOD'] == "POST") {
    switch ($_POST["action"]) {
      case "get_node_list":
        action_get_node_list();
        break;
      case "entry":
        action_entry(
          $_POST["name"],
          $_POST["addr"],
          $_POST["port"],
          $_POST["busy"]);
        break;
    }
  }
  print("##foot##\n");
}

function action_get_node_list() {
  $node_list = file(NODE_LIST_FILE);
  foreach($node_list as $node){
    print($node);
  }
}

function action_entry($name, $addr, $port, $busy) {
  mkdir(LOCK_DIR_NAME, 0755);
  
  $fp = fopen(NODE_LIST_FILE, "a+");
  fwrite($fp, "name=".$name." addr=".$addr." port=".$port." busy=".$busy."\n");
  fclose($fp);
  
  rmdir(LOCK_DIR_NAME);
}

?>
