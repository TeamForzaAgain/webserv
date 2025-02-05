/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tpicchio <tpicchio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 17:09:44 by tpicchio          #+#    #+#             */
/*   Updated: 2025/02/05 14:50:43 by tpicchio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Include.hpp"

HttpResponse genResponse(int statusCode, ServerConfig const &serverConfig, Route const &route)
{
	HttpResponse response;

	response.setStatusCode(statusCode);
	response.setServerName(serverConfig.serverName);
	response.setServerVersion(serverConfig.serverVersion);
	response.setRoute(route);

	return response;
}
