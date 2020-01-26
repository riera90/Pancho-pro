.PHONY : run
run : ./app.py setup
	./.env/bin/python ./app.py


.PHONY : setup
setup : ./.env/bin/activate
	python3 -m virtualenv -p /usr/bin/python3 .env
	./.env/bin/python -m pip install -r requirements.txt