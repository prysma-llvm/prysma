
import re
def to_snake_case(name):
    if '.' in name:
        base, ext = name.rsplit('.', 1)
        s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', base)
        s2 = re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()
        return f"{re.sub(r'[\s\-]+', '_', s2).strip('_')}.{ext.lower()}"
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    s2 = re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()
    return re.sub(r'[\s\-]+', '_', s2).strip('_')
import yaml
import os
from jinja2 import Environment, FileSystemLoader


class EngineGeneration:

    def __init__(self, racine_project):
        self._racine = racine_project
        self._fichier_yaml = os.path.join(racine_project, "include", "compiler", "ast", "yaml_ast", "ast.yaml")
        self._generation_code = os.path.join(racine_project, "build", "generationCode")
        self._generation_include = os.path.join(self._generation_code, "include")
        self._generation_src = os.path.join(self._generation_code, "src")
        self._env = Environment(
            loader=FileSystemLoader(os.path.join(racine_project, "generation", "templates")),
            trim_blocks=True,
            lstrip_blocks=True
        )

    def _loadr_nodes_yaml(self):
        with open(self._fichier_yaml, "r", encoding="utf-8") as fichier:
            nodes = yaml.safe_load(fichier).get("Node", {})
        return self._applatir_nodes(nodes)

    def _loadr_definitions_nodes(self):
        return self._loadr_nodes_yaml()

    def _loadr_nodes_yaml_par_categories(self):
        with open(self._fichier_yaml, "r", encoding="utf-8") as fichier:
            return yaml.safe_load(fichier).get("Node", {})
        
    def _applatir_nodes(self, nodes):
        nodes_applatir = {}
        for _, donnees in nodes.items():
            nodes_applatir.update(donnees)
           
        return nodes_applatir
    
    def _rendre_et_ecrire(self, nom_template, chemin_output, **kwargs):
        contenu = self._env.get_template(nom_template).render(**kwargs)
        os.makedirs(os.path.dirname(chemin_output), exist_ok=True)
        # Only write if content actually changed, to preserve timestamps for CMake/ccache
        if os.path.exists(chemin_output):
            with open(chemin_output, "r", encoding="utf-8") as fichier:
                if fichier.read() == contenu:
                    return  # Content identical, skip to preserve timestamp
        with open(chemin_output, "w", encoding="utf-8") as fichier:
            fichier.write(contenu)

    def _rendre_et_ecrire_si_absent(self, nom_template, chemin_output, **kwargs):
        if os.path.exists(chemin_output):
            return False
        self._rendre_et_ecrire(nom_template, chemin_output, **kwargs)
        return True

    def _chemin_generation_include(self, *segments):
        return os.path.join(self._generation_include, *segments)

    def _chemin_generation_src(self, *segments):
        return os.path.join(self._generation_src, *segments)

    def _chemin_depuis_categorie(self, categorie, *segments):
        return os.path.join("compiler", categorie, *segments)

    def _construire_mapping_node_categorie(self):
        mapping = {}
        for categorie, nodes in self._loadr_nodes_yaml_par_categories().items():
            for nom_node in (nodes or {}).keys():
                mapping[nom_node] = categorie
        return mapping

    def _resoudre_categorie_node(self, nom_node, donnees_node=None, cle_override=None, categorie_defaut="parser"):
        if donnees_node is not None and cle_override is not None and donnees_node.get(cle_override):
            return donnees_node[cle_override]

        definitions = self._loadr_definitions_nodes()
        definition = donnees_node or definitions.get(nom_node, {})

        if cle_override is not None and definition.get(cle_override):
            return definition[cle_override]

        return self._construire_mapping_node_categorie().get(nom_node, categorie_defaut)

    @staticmethod
    def _normaliser_engines(donnees_node):
        engines = donnees_node.get("engine", ["parser"])
        if isinstance(engines, str):
            return [engines]
        return list(engines)

    def _a_engine(self, donnees_node, engine):
        return engine in self._normaliser_engines(donnees_node)

    @staticmethod
    def _extraire_traversables(champs):
        resultats = []
        for nom_champ, type_champ in champs.items():
            if type_champ in ("INode*", "llvm::ArrayRef<INode*>"):
                getter = "get" + nom_champ[0].upper() + nom_champ[1:]
                resultats.append((getter, type_champ))
        return resultats
