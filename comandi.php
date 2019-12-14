<html>
	<head>
		<title>ricezione dati</title>
	</head>
	<body>



	<?php
        // http://yourSite/comandi.php?cmd=add_riga&start_time=1&end_time=2&start_distance=3&end_distance=4
        // http://yourSite/comandi.php?cmd=lista

		/* il primo argomento $cmd e' il comando
            add_riga:	mette la riga corrente nel db
            lista_file:
        */
		if ($_SERVER["REQUEST_METHOD"] == "GET") {
			$cmd   = test_input($_GET["cmd"]);
		}
        echo "cmd: " . $cmd ."<br>";

		$servername = "localhost";
        $username   = "";
        $password   = "";
        $dbname     = "";        
		
        if ($cmd == "add_riga"){		
        	if ($_SERVER["REQUEST_METHOD"] == "GET") {
				$sender_id      = 0;			// id di chi invia i dati
                $start_time		=  test_input($_GET["start_time"]);
                $end_time  		=  test_input($_GET["end_time"]);
                $start_distance =  test_input($_GET["start_distance"]);
                $end_distance 	=  test_input($_GET["end_distance"]);

              // Create connection
              $conn = new mysqli($servername, $username, $password, $dbname);
              // Check connection
              if ($conn->connect_error) {
                  die("Connection failed: " . $conn->connect_error);
              }

              $sql = "INSERT INTO `ciclista_table`( `start_time`, `end_time`, `start_distance`, `end_distance`, `senderID`) 
                      VALUES (". $start_time .",". $end_time .",". $start_distance .",". $end_distance .",". $sender_id .")";

              if ($conn->query($sql) === TRUE) {
                  echo "New record created successfully";
              } else {
                  echo "Error: " . $sql . "<br>" . $conn->error;
              }

              $conn->close();              
            }
		}


		if ($cmd == "lista"){	
        	
            // Create connection
            $conn = new mysqli($servername, $username, $password, $dbname);
            // Check connection
            if ($conn->connect_error) {
                die("Connection failed: " . $conn->connect_error);
            }

 $sql = "SELECT `id`, `date_time`, `start_time`, `end_time`, `start_distance`, `end_distance`, `senderID` FROM `ciclista_table` WHERE 1 ";
 //           $sql = "SELECT id, date_time, start_time, end_time, start_distance, end_distance, senderID FROM ciclista_table WHERE 1";
            $result = $conn->query($sql);

            if ($result->num_rows > 0) {
                // output data of each row
                while($row = $result->fetch_assoc()) {
                    echo "id: " . $row["id"]. "; " .$row["date_time"]. "; " .$row["start_time"].",". $row[end_time] .",". $row[start_distance] .",". $row[end_distance] .",". $row[senderID] . "<br>";
                }
            } else {
                echo "0 results <br>";
            }
$conn->close();       
        }

		function test_input($data) {
			$data = trim($data);
			$data = stripslashes($data);
			$data = htmlspecialchars($data);
			return $data;
		}
	?>
		
	</body>
</html>
