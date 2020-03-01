import kappy

# Modify the path according to the location of the kappa agent on your system
p_thibaut =  "../../Kappapp/resources/bin"

path_to_agent = p_thibaut

#creation du client
client = kappy.KappaStd(path_to_agent)

#choice of model
f = open("urm.ka", "r")
model = f.read()
f.close()

client.add_model_string(model)
client.project_parse()


#do stuff


client.shutdown()
