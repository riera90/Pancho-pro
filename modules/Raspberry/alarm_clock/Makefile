.PHONY : run
run : ./app.py setup
	./.venv/bin/python ./app.py


.PHONY : setup
setup : 
	python3 -m virtualenv -p /usr/bin/python3 .venv
	./.venv/bin/python -m pip install -r requirements.txt
