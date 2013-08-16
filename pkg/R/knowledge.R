#Condition
setClass("crules.condition", representation(
				attribute = "character", value = "character", from = "numeric", to = "numeric", 
				fixed = "logical", required = "logical"))

condition <- function(attribute, value = "", from = -Inf, to = Inf, fixed = FALSE, required = FALSE){
				 
	isNumeric <- value == "";
	
	if(isNumeric && from > to)
		stop("The value of parameter 'from' should be less than value of parameter 'to'");
	
	new("crules.condition", attribute = attribute, value = as.character(value), from = from, to = to, 
			fixed = fixed, required = required)
}

#Conditions
setClass("crules.conditions", representation(setOfConditions = "list", className = "character", expandable = "logical", rulesAtLeast = "numeric", forbidden="logical"));

conditions <- function(..., className, expandable = TRUE, rulesAtLeast = 0, forbidden = FALSE){
	setOfConditions <- list(...);
	if(!.areAllElementsOfClass(setOfConditions, "crules.condition"))
		stop("Elements have to be of class 'crules.condition'");
	
	new("crules.conditions", setOfConditions = setOfConditions, className = as.character(className), 
		expandable = expandable, rulesAtLeast = rulesAtLeast, forbidden = forbidden);
}

#Rule
setClass("crules.rule", representation(conditions = "crules.conditions"))

rule <- function(conditions){	
	new ("crules.rule", conditions = conditions)
}

##Rules
#setClass("crules.rules", representation(listOfRules = "list"));
#
#rules <- function(listOfRules){
#	new ("crules.rules", listOfRules = listOfRules)
#}

#Knowledge
setClass("crules.knowledge", representation(generateRulesForOtherClasses  = "logical",
				useSpecifiedOnly = "logical", requirements = "list"))

#structure of "requirements" list:
#requirements[className] - length equal to number of classes
#four members: 
#- forbiddenRules, allowedRules - lists of crules.rule class objects 
#- forbiddenConditions, allowedConditions - crules.conditions class objects

setMethod("c", c("crules.knowledge", "ANY"),
		function(x, ...){
			added <- list(...);
			
			rules <- .getElementsOfClass(added, "crules.rule");
			rulesLength = length(rules)
			conditionsSets <- .getElementsOfClass(added, "crules.conditions");
			conditionsSetsLength <- length(conditionsSets)
			
			if(length(added) > rulesLength + conditionsSetsLength)
				stop("Only crules.rule and crules.conditions classes can be added to 'knowledge'")
			
			if(rulesLength > 0){
				for(i in 1:rulesLength){
					conditions <- rules[[i]]@conditions
					if(is.null(x@requirements[[conditions@className]]))
						x@requirements[[conditions@className]] <- list()
					
					classReqs <- x@requirements[[conditions@className]]
					
					if(conditions@forbidden)
						member <- "forbiddenRules"
					else
						member <- "allowedRules"
	
					if(is.null(classReqs[[member]]))
						x@requirements[[conditions@className]][[member]] <- list();
					
					x@requirements[[conditions@className]][[member]] <- c(x@requirements[[conditions@className]][[member]], rules[[i]])
					
				}
			}
			
			if(conditionsSetsLength > 0){
				for(i in 1:conditionsSetsLength){
					conditions <- conditionsSets[[i]]
					if(is.null(x@requirements[[conditions@className]]))
						x@requirements[[conditions@className]] <- list()
					
					classReqs <- x@requirements[[conditions@className]]
					
					if(conditions@forbidden)
						member <- "forbiddenConditions"
					else
						member <- "allowedConditions"
					
					if(!is.null(classReqs[[member]]))
						print("Warning: there is already a similar requirement defined. It will be replaced.")
					
					x@requirements[[conditions@className]][[member]] <- conditions
					
				}
			}
			x;
		})


knowledge <- function(..., generateRulesForOtherClasses = FALSE, useSpecifiedOnly = TRUE){
	newObject <- new("crules.knowledge", generateRulesForOtherClasses = generateRulesForOtherClasses,
			useSpecifiedOnly = useSpecifiedOnly);
	
	if(length(list(...)) > 0)
		newObject <- c(newObject, ...);
	newObject;
}

.areAllElementsOfClass <- function(l, className){
	all(sapply(l, function(x) class(x) == className))
}

.getElementsOfClass <- function(l, className){
	l[sapply(l, function(x) class(x) == className)]
}