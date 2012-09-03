/******************************************************************************

 This source file is part of the MoleQueue project.

 Copyright 2012 Kitware, Inc.

 This source code is released under the New BSD License, (the "License").

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 ******************************************************************************/

#include "authenticateresponse.h"
#include "logger.h"

#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QAbstractMessageHandler>

namespace MoleQueue
{

/**
 * Concrete QAbstractMessageHandler implementation used to report errors
 * associated with parsing XML content past QXmlQuery objects.
 */
class MessageHandler : public QAbstractMessageHandler
{
public:
  MessageHandler(QObject *parentObject = 0);

protected:
  virtual void handleMessage(QtMsgType type, const QString &description,
                             const QUrl &identifier,
                             const QSourceLocation &sourceLocation);
};


MessageHandler::MessageHandler(QObject *parentObject)
  : QAbstractMessageHandler(parentObject)
{

}

void MessageHandler::handleMessage(QtMsgType type, const QString &description,
                                   const QUrl &identifier,
                                   const QSourceLocation &sourceLocation)
{
  Q_UNUSED(type);
  Q_UNUSED(identifier);
  Q_UNUSED(sourceLocation);
  Logger::logError(description);
}

Prompt::Prompt(int i, const QString p)
  : m_id(i), m_prompt(p)
{

}

Prompt::Prompt(const Prompt &p)
  : m_id(p.id()), m_prompt(p.prompt()), m_userResponse(p.userResponse())
{

}

AuthenticateResponse::AuthenticateResponse()
  : m_hasPrompts(false), m_success(false), m_valid(false)
{

}

AuthenticateResponse::AuthenticateResponse(const AuthenticateResponse &response)
 : m_authSessionId(response.authSessionId()),
   m_hasPrompts(response.hasPrompts()), m_prompts(response.prompts()),
   m_success(response.success()), m_errorMessage(response.errorMessage()),
   m_banner(response.banner()),m_token(response.token()),
   m_valid(response.isValid())
{

}

AuthenticateResponse &AuthenticateResponse::operator=(
  const AuthenticateResponse &other)
{
  if (this != &other) {
    m_authSessionId = other.authSessionId();
    m_hasPrompts = other.hasPrompts();
    m_prompts = other.prompts();
    m_success = other.success();
    m_errorMessage = other.errorMessage();
    m_banner = other.banner();
    m_token = other.token();
    m_valid = other.isValid();
  }

  return *this;
}

void AuthenticateResponse::setContent(const QString &xml)
{
  m_valid = true;

  MessageHandler handler;
  QXmlQuery query;
  query.setMessageHandler(&handler);
  m_valid = query.setFocus(xml);

  if(!m_valid)
    return;

  // Get the session id
  QString authId;
  query.setQuery("/AuthenticateResponse/auth__session__id/string()");
  query.evaluateTo(&authId);
  m_authSessionId = authId.trimmed();

  // Was the call successful
  QString successful;
  query.setQuery("/AuthenticateResponse/success/string()");
  m_valid = query.evaluateTo(&successful);
  if(!m_valid)
    return;

  m_success = successful.trimmed().toLower() == "true";

  // Do we have prompts
  QString hasProm;
  query.setQuery("/AuthenticateResponse/has__prompts/string()");
  m_valid = query.evaluateTo(&hasProm);
  if(!m_valid)
    return;

  m_hasPrompts = hasProm.trimmed().toLower() == "true";

  // Get the banner
  QString ban;
  query.setQuery("/AuthenticateResponse/banner/string()");
  m_valid = query.evaluateTo(&ban);
  if(!m_valid)
    return;

  m_banner = ban.trimmed();

  // Get the token, if there is one
  QString tok;
  query.setQuery("/AuthenticateResponse/token/string()");
  m_valid = query.evaluateTo(&tok);
  if(!m_valid)
    return;

  m_token = tok.trimmed();

  // if we have prompts then get them
  if(m_hasPrompts) {
    query.setQuery("/AuthenticateResponse/prompts/Prompt/id/string()");
    QStringList ids;
    m_valid = query.evaluateTo(&ids);
    if(!m_valid)
      return;


    foreach (const QString &id, ids) {
      query.bindVariable("id", QVariant(id));
      query.setQuery(QString("/AuthenticateResponse/prompts/Prompt[id=$id]/" \
                             "prompt/string()"));
      QString prompt;
      m_valid = query.evaluateTo(&prompt);
      if(!m_valid)
        return;

      m_prompts.append(Prompt(id.toInt(), prompt.trimmed()));
    }
  }

  QString error;
  query.setQuery("/AuthenticateResponse/error__message/string()");
  m_valid = query.evaluateTo(&error);
  m_errorMessage = error.trimmed();
}

QString AuthenticateResponse::authSessionId() const
{
  return m_authSessionId;
}

bool AuthenticateResponse::hasPrompts() const
{
  return m_hasPrompts;
}

QList<Prompt> AuthenticateResponse::prompts() const
{
  return m_prompts;

}
bool AuthenticateResponse::success() const
{
  return m_success;
}

QString AuthenticateResponse::errorMessage() const
{
  return m_errorMessage;
}

QString AuthenticateResponse::banner() const
{
  return m_banner;
}

QString AuthenticateResponse::token() const
{
  return m_token;
}

bool AuthenticateResponse::isValid() const
{
  return m_valid;
}

AuthenticateResponse AuthenticateResponse::fromXml(const QString &xml)
{
  AuthenticateResponse  response;
  response.setContent(xml);

  return response;
}

} /* namespace MoleQueue */
