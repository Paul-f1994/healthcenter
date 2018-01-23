com:
	make -f makefle_doctor clean
	make -f makefle_doctor
	make -f makefle_healthcenter clean
	make -f makefle_healthcenter
	make -f makefle_patient clean
	make -f makefle_patient
clean:
	make -f makefle_doctor clean
	make -f makefle_healthcenter clean
	make -f makefle_patient clean
	
